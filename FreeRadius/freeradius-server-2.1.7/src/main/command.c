/*
 * command.c	Command socket processing.
 *
 * Version:	$Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2008 The FreeRADIUS server project
 * Copyright 2008 Alan DeKok <aland@deployingradius.com>
 */

#ifdef WITH_COMMAND_SOCKET

#include <freeradius-devel/modpriv.h>
#include <freeradius-devel/conffile.h>
#include <freeradius-devel/stats.h>
#include <freeradius-devel/realms.h>

#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#ifndef SUN_LEN
#define SUN_LEN(su)  (sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))
#endif
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#ifdef HAVE_GRP_H
#include <grp.h>
#endif

typedef struct fr_command_table_t fr_command_table_t;

typedef int (*fr_command_func_t)(rad_listen_t *, int, char *argv[]);

#define FR_READ  (1)
#define FR_WRITE (2)

struct fr_command_table_t {
	const char *command;
	int mode;		/* read/write */
	const char *help;
	fr_command_func_t func;
	fr_command_table_t *table;
};

#define COMMAND_BUFFER_SIZE (1024)

typedef struct fr_command_socket_t {
	char	*path;
	uid_t	uid;
	gid_t	gid;
	int	mode;
	char	*uid_name;
	char	*gid_name;
	char	*mode_name;
	char user[256];

	/*
	 *	The next few entries handle fake packets injected by
	 *	the control socket.
	 */
	fr_ipaddr_t	src_ipaddr; /* src_port is always 0 */
	fr_ipaddr_t	dst_ipaddr;
	int		dst_port;
	rad_listen_t	*inject_listener;
	RADCLIENT	*inject_client;

	/*
	 *	The next few entries do buffer management.
	 */
	ssize_t offset;
	ssize_t next;
	char buffer[COMMAND_BUFFER_SIZE];
} fr_command_socket_t;

static const CONF_PARSER command_config[] = {
  { "socket",  PW_TYPE_STRING_PTR,
    offsetof(fr_command_socket_t, path), NULL, "${run_dir}/radiusd.sock"},
  { "uid",  PW_TYPE_STRING_PTR,
    offsetof(fr_command_socket_t, uid_name), NULL, NULL},
  { "gid",  PW_TYPE_STRING_PTR,
    offsetof(fr_command_socket_t, gid_name), NULL, NULL},
  { "mode",  PW_TYPE_STRING_PTR,
    offsetof(fr_command_socket_t, mode_name), NULL, NULL},

  { NULL, -1, 0, NULL, NULL }		/* end the list */
};

static FR_NAME_NUMBER mode_names[] = {
	{ "ro", FR_READ },
	{ "read-only", FR_READ },
	{ "read-write", FR_READ | FR_WRITE },
	{ "rw", FR_READ | FR_WRITE },
	{ NULL, 0 }
};


static ssize_t cprintf(rad_listen_t *listener, const char *fmt, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 2, 3)))
#endif
;

#ifndef HAVE_GETPEEREID
static int getpeereid(int s, uid_t *euid, gid_t *egid)
{
#ifndef SO_PEERCRED
	return -1;
#else
	struct ucred cr;
	socklen_t cl = sizeof(cr);
	
	if (getsockopt(s, SOL_SOCKET, SO_PEERCRED, &cr, &cl) < 0) {
		return -1;
	}

	*euid = cr.uid;
	*egid = cr.gid;
	return 0;
#endif /* SO_PEERCRED */
}
#endif /* HAVE_GETPEEREID */


static int fr_server_domain_socket(const char *path)
{
        int sockfd;
	size_t len;
	socklen_t socklen;
        struct sockaddr_un salocal;
	struct stat buf;

	len = strlen(path);
	if (len >= sizeof(salocal.sun_path)) {
		radlog(L_ERR, "Path too long in socket filename.");
		return -1;
	}

        if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		radlog(L_ERR, "Failed creating socket: %s",
			strerror(errno));
		return -1;
        }

	memset(&salocal, 0, sizeof(salocal));
        salocal.sun_family = AF_UNIX;
	memcpy(salocal.sun_path, path, len + 1); /* SUN_LEN does strlen */
	
	socklen = SUN_LEN(&salocal);

	/*
	 *	Check the path.
	 */
	if (stat(path, &buf) < 0) {
		if (errno != ENOENT) {
			radlog(L_ERR, "Failed to stat %s: %s",
			       path, strerror(errno));
			return -1;
		}

		/*
		 *	FIXME: Check the enclosing directory?
		 */
	} else {		/* it exists */
		if (!S_ISREG(buf.st_mode)
#ifdef S_ISSOCK
		    && !S_ISSOCK(buf.st_mode)
#endif
			) {
			radlog(L_ERR, "Cannot turn %s into socket", path);
			return -1;		       
		}

		/*
		 *	Refuse to open sockets not owned by us.
		 */
		if (buf.st_uid != geteuid()) {
			radlog(L_ERR, "We do not own %s", path);
			return -1;
		}

		if (unlink(path) < 0) {
			radlog(L_ERR, "Failed to delete %s: %s",
			       path, strerror(errno));
			return -1;
		}
	}

        if (bind(sockfd, (struct sockaddr *)&salocal, socklen) < 0) {
		radlog(L_ERR, "Failed binding to %s: %s",
			path, strerror(errno));
		close(sockfd);
		return -1;
        }

	/*
	 *	FIXME: There's a race condition here.  But Linux
	 *	doesn't seem to permit fchmod on domain sockets.
	 */
	if (chmod(path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) < 0) {
		radlog(L_ERR, "Failed setting permissions on %s: %s",
		       path, strerror(errno));
		close(sockfd);
		return -1;
	}

	if (listen(sockfd, 8) < 0) {
		radlog(L_ERR, "Failed listening to %s: %s",
			path, strerror(errno));
		close(sockfd);
		return -1;
        }

#ifdef O_NONBLOCK
	{
		int flags;
		
		if ((flags = fcntl(sockfd, F_GETFL, NULL)) < 0)  {
			radlog(L_ERR, "Failure getting socket flags: %s",
				strerror(errno));
			close(sockfd);
			return -1;
		}
		
		flags |= O_NONBLOCK;
		if( fcntl(sockfd, F_SETFL, flags) < 0) {
			radlog(L_ERR, "Failure setting socket flags: %s",
				strerror(errno));
			close(sockfd);
			return -1;
		}
	}
#endif

	return sockfd;
}

static void command_close_socket(rad_listen_t *this)
{
	this->status = RAD_LISTEN_STATUS_CLOSED;

	/*
	 *	This removes the socket from the event fd, so no one
	 *	will be calling us any more.
	 */
	event_new_fd(this);

	listen_free(&this);
}


static ssize_t cprintf(rad_listen_t *listener, const char *fmt, ...)
{
	ssize_t len;
	va_list ap;
	char buffer[256];

	va_start(ap, fmt);
	len = vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	if (listener->status == RAD_LISTEN_STATUS_CLOSED) return 0;

	len = write(listener->fd, buffer, len);
	if (len <= 0) command_close_socket(listener);

	/*
	 *	FIXME: Keep writing until done?
	 */
	return len;
}

static int command_hup(rad_listen_t *listener, int argc, char *argv[])
{
	CONF_SECTION *cs;
	module_instance_t *mi;

	if (argc == 0) {
		radius_signal_self(RADIUS_SIGNAL_SELF_HUP);
		return 1;
	}

	cs = cf_section_find("modules");
	if (!cs) return 0;

	mi = find_module_instance(cs, argv[0], 0);
	if (!mi) {
		cprintf(listener, "ERROR: No such module \"%s\"\n", argv[0]);
		return 0;
	}

	if ((mi->entry->module->type & RLM_TYPE_HUP_SAFE) == 0) {
		cprintf(listener, "ERROR: Module %s cannot be hup'd\n",
			argv[0]);
		return 0;
	}

	if (!module_hup_module(mi->cs, mi, time(NULL))) {
		cprintf(listener, "ERROR: Failed to reload module\n");
		return 0;
	}

	return 1;		/* success */
}

static int command_terminate(UNUSED rad_listen_t *listener,
			     UNUSED int argc, UNUSED char *argv[])
{
	radius_signal_self(RADIUS_SIGNAL_SELF_TERM);

	return 1;		/* success */
}

extern time_t fr_start_time;

static int command_uptime(rad_listen_t *listener,
			  UNUSED int argc, UNUSED char *argv[])
{
	char buffer[128];

	CTIME_R(&fr_start_time, buffer, sizeof(buffer));
	cprintf(listener, "Up since %s", buffer); /* no \r\n */

	return 1;		/* success */
}

static const char *tabs = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

/*
 *	FIXME: Recurse && indent?
 */
static void cprint_conf_parser(rad_listen_t *listener, int indent, CONF_SECTION *cs,
			       const void *base)
			       
{
	int i;
	const void *data;
	const char *name1 = cf_section_name1(cs);
	const char *name2 = cf_section_name2(cs);
	const CONF_PARSER *variables = cf_section_parse_table(cs);
	char buffer[256];

	if (name2) {
		cprintf(listener, "%.*s%s %s {\n", indent, tabs, name1, name2);
	} else {
		cprintf(listener, "%.*s%s {\n", indent, tabs, name1);
	}

	indent++;
	
	/*
	 *	Print
	 */
	if (variables) for (i = 0; variables[i].name != NULL; i++) {
		/*
		 *	No base struct offset, data must be the pointer.
		 *	If data doesn't exist, ignore the entry, there
		 *	must be something wrong.
		 */
		if (!base) {
			if (!variables[i].data) {
				continue;
			}
			
			data = variables[i].data;;
			
		} else if (variables[i].data) {
			data = variables[i].data;;
			
		} else {
			data = (((char *)base) + variables[i].offset);
		}

		switch (variables[i].type) {
		default:
			cprintf(listener, "%.*s%s = ?\n", indent, tabs,
				variables[i].name);
			break;
			
		case PW_TYPE_INTEGER:
			cprintf(listener, "%.*s%s = %u\n", indent, tabs,
				variables[i].name, *(int *) data);
			break;
			
		case PW_TYPE_IPADDR:
			inet_ntop(AF_INET, data, buffer, sizeof(buffer));
			break;

		case PW_TYPE_IPV6ADDR:
			inet_ntop(AF_INET6, data, buffer, sizeof(buffer));
			break;

		case PW_TYPE_BOOLEAN:
			cprintf(listener, "%.*s%s = %s\n", indent, tabs,
				variables[i].name, 
				((*(int *) data) == 0) ? "no" : "yes");
			break;
			
		case PW_TYPE_STRING_PTR:
		case PW_TYPE_FILENAME:
			/*
			 *	FIXME: Escape things in the string!
			 */
			if (*(char **) data) {
				cprintf(listener, "%.*s%s = \"%s\"\n", indent, tabs,
					variables[i].name, *(char **) data);
			} else {
				cprintf(listener, "%.*s%s = \n", indent, tabs,
					variables[i].name);
			}
				
			break;
		}
	}

	indent--;

	cprintf(listener, "%.*s}\n", indent, tabs);
}

static int command_show_module_config(rad_listen_t *listener, int argc, char *argv[])
{
	CONF_SECTION *cs;
	module_instance_t *mi;

	if (argc != 1) {
		cprintf(listener, "ERROR: No module name was given\n");
		return 0;
	}

	cs = cf_section_find("modules");
	if (!cs) return 0;

	mi = find_module_instance(cs, argv[0], 0);
	if (!mi) {
		cprintf(listener, "ERROR: No such module \"%s\"\n", argv[0]);
		return 0;
	}

	cprint_conf_parser(listener, 0, mi->cs, mi->insthandle);

	return 1;		/* success */
}

static const char *method_names[RLM_COMPONENT_COUNT] = {
	"authenticate",
	"authorize",
	"preacct",
	"accounting",
	"session",
	"pre-proxy",
	"post-proxy",
	"post-auth"
};


static int command_show_module_methods(rad_listen_t *listener, int argc, char *argv[])
{
	int i;
	CONF_SECTION *cs;
	const module_instance_t *mi;
	const module_t *mod;

	if (argc != 1) {
		cprintf(listener, "ERROR: No module name was given\n");
		return 0;
	}

	cs = cf_section_find("modules");
	if (!cs) return 0;

	mi = find_module_instance(cs, argv[0], 0);
	if (!mi) {
		cprintf(listener, "ERROR: No such module \"%s\"\n", argv[0]);
		return 0;
	}

	mod = mi->entry->module;

	for (i = 0; i < RLM_COMPONENT_COUNT; i++) {
		if (mod->methods[i]) cprintf(listener, "\t%s\n", method_names[i]);
	}

	return 1;		/* success */
}


static int command_show_module_flags(rad_listen_t *listener, int argc, char *argv[])
{
	CONF_SECTION *cs;
	const module_instance_t *mi;
	const module_t *mod;

	if (argc != 1) {
		cprintf(listener, "ERROR: No module name was given\n");
		return 0;
	}

	cs = cf_section_find("modules");
	if (!cs) return 0;

	mi = find_module_instance(cs, argv[0], 0);
	if (!mi) {
		cprintf(listener, "ERROR: No such module \"%s\"\n", argv[0]);
		return 0;
	}

	mod = mi->entry->module;

	if ((mod->type & RLM_TYPE_THREAD_SAFE) != 0)
		cprintf(listener, "\tthread-safe\n");


	if ((mod->type & RLM_TYPE_CHECK_CONFIG_SAFE) != 0)
		cprintf(listener, "\twill-check-config\n");


	if ((mod->type & RLM_TYPE_HUP_SAFE) != 0)
		cprintf(listener, "\treload-on-hup\n");

	return 1;		/* success */
}


/*
 *	Show all loaded modules
 */
static int command_show_modules(rad_listen_t *listener, UNUSED int argc, UNUSED char *argv[])
{
	CONF_SECTION *cs, *subcs;

	cs = cf_section_find("modules");
	if (!cs) return 0;

	subcs = NULL;
	while ((subcs = cf_subsection_find_next(cs, subcs, NULL)) != NULL) {
		const char *name1 = cf_section_name1(subcs);
		const char *name2 = cf_section_name2(subcs);

		module_instance_t *mi;

		if (name2) {
			mi = find_module_instance(cs, name2, 0);
			if (!mi) continue;

			cprintf(listener, "\t%s (%s)\n", name2, name1);
		} else {
			mi = find_module_instance(cs, name1, 0);
			if (!mi) continue;

			cprintf(listener, "\t%s\n", name1);
		}
	}

	return 1;		/* success */
}

#ifdef WITH_PROXY
static int command_show_home_servers(rad_listen_t *listener, UNUSED int argc, UNUSED char *argv[])
{
	int i;
	home_server *home;
	const char *type, *state;

	char buffer[256];

	for (i = 0; i < 256; i++) {
		home = home_server_bynumber(i);
		if (!home) break;

		/*
		 *	Internal "virtual" home server.
		 */
		if (home->ipaddr.af == AF_UNSPEC) continue;

		if (home->type == HOME_TYPE_AUTH) {
			type = "auth";

		} else if (home->type == HOME_TYPE_ACCT) {
			type = "acct";

		} else continue;

		if (home->state == HOME_STATE_ALIVE) {
			state = "alive";

		} else if (home->state == HOME_STATE_ZOMBIE) {
			state = "zombie";

		} else if (home->state == HOME_STATE_IS_DEAD) {
			state = "dead";

		} else continue;

		cprintf(listener, "%s\t%d\t%s\t%s\t%d\n",
			ip_ntoh(&home->ipaddr, buffer, sizeof(buffer)),
			home->port, type, state,
			home->currently_outstanding);
	}

	return 0;
}
#endif

static int command_show_clients(rad_listen_t *listener, UNUSED int argc, UNUSED char *argv[])
{
	int i;
	RADCLIENT *client;
	char buffer[256];

	for (i = 0; i < 256; i++) {
		client = client_findbynumber(NULL, i);
		if (!client) break;

		ip_ntoh(&client->ipaddr, buffer, sizeof(buffer));

		if (((client->ipaddr.af == AF_INET) &&
		     (client->prefix != 32)) ||
		    ((client->ipaddr.af == AF_INET6) &&
		     (client->prefix != 128))) {
			cprintf(listener, "\t%s/%d\n", buffer, client->prefix);
		} else {
			cprintf(listener, "\t%s\n", buffer);
		}
	}

	return 0;
}


static int command_show_xml(rad_listen_t *listener, UNUSED int argc, UNUSED char *argv[])
{
	CONF_ITEM *ci;
	FILE *fp = fdopen(dup(listener->fd), "a");

	if (!fp) {
		cprintf(listener, "ERROR: Can't dup %s\n", strerror(errno));
		return 0;
	}

	if (argc == 0) {
		cprintf(listener, "ERROR: <reference> is required\n");
		return 0;
	}
	
	ci = cf_reference_item(mainconfig.config, mainconfig.config, argv[0]);
	if (!ci) {
		cprintf(listener, "ERROR: No such item <reference>\n");
		return 0;
	}

	if (cf_item_is_section(ci)) {
		cf_section2xml(fp, cf_itemtosection(ci));

	} else if (cf_item_is_pair(ci)) {
		cf_pair2xml(fp, cf_itemtopair(ci));

	} else {
		cprintf(listener, "ERROR: No such item <reference>\n");
		fclose(fp);
		return 0;
	}

	fclose(fp);

	return 1;		/* success */
}

static int command_show_version(rad_listen_t *listener, UNUSED int argc, UNUSED char *argv[])
{
	cprintf(listener, "%s\n", radiusd_version);
	return 1;
}

static int command_debug_level(rad_listen_t *listener, int argc, char *argv[])
{
	int number;

	if (argc == 0) {
		cprintf(listener, "ERROR: Must specify <number>\n");
		return -1;
	}

	number = atoi(argv[0]);
	if ((number < 0) || (number > 4)) {
		cprintf(listener, "ERROR: <number> must be between 0 and 4\n");
		return -1;
	}

	fr_debug_flag = debug_flag = number;

	return 0;
}

char *debug_log_file = NULL;
static char debug_log_file_buffer[1024];

static int command_debug_file(rad_listen_t *listener, int argc, char *argv[])
{
	if (debug_flag && mainconfig.radlog_dest == RADLOG_STDOUT) {
		cprintf(listener, "ERROR: Cannot redirect debug logs to a file when already in debugging mode.\n");
		return -1;
	}

	if ((argc > 0) && (strchr(argv[0], FR_DIR_SEP) != NULL)) {
		cprintf(listener, "ERROR: Cannot direct debug logs to absolute path.\n");
	}

	debug_log_file = NULL;

	if (argc == 0) return 0;

	/*
	 *	This looks weird, but it's here to avoid locking
	 *	a mutex for every log message.
	 */
	memset(debug_log_file_buffer, 0, sizeof(debug_log_file_buffer));

	/*
	 *	Debug files always go to the logging directory.
	 */
	snprintf(debug_log_file_buffer, sizeof(debug_log_file_buffer),
		 "%s/%s", radlog_dir, argv[0]);

	debug_log_file = &debug_log_file_buffer[0];

	return 0;
}

extern char *debug_condition;
static int command_debug_condition(UNUSED rad_listen_t *listener, int argc, char *argv[])
{
	/*
	 *	Delete old condition.
	 *
	 *	This is thread-safe because the condition is evaluated
	 *	in the main server thread, as is this code.
	 */
	free(debug_condition);
	debug_condition = NULL;

	/*
	 *	Disable it.
	 */
	if (argc == 0) {
		return 0;
	}

	debug_condition = strdup(argv[0]);

	return 0;
}

static int command_show_debug_condition(rad_listen_t *listener,
					UNUSED int argc, UNUSED char *argv[])
{
	if (!debug_condition) return 0;

	cprintf(listener, "%s\n", debug_condition);
	return 0;
}


static int command_show_debug_file(rad_listen_t *listener,
					UNUSED int argc, UNUSED char *argv[])
{
	if (!debug_log_file) return 0;

	cprintf(listener, "%s\n", debug_log_file);
	return 0;
}


static int command_show_debug_level(rad_listen_t *listener,
					UNUSED int argc, UNUSED char *argv[])
{
	cprintf(listener, "%d\n", debug_flag);
	return 0;
}


static RADCLIENT *get_client(rad_listen_t *listener, int argc, char *argv[])
{
	RADCLIENT *client;
	fr_ipaddr_t ipaddr;

	if (argc < 1) {
		cprintf(listener, "ERROR: Must specify <ipaddr>\n");
		return NULL;
	}

	if (ip_hton(argv[0], AF_UNSPEC, &ipaddr) < 0) {
		cprintf(listener, "ERROR: Failed parsing IP address; %s\n",
			fr_strerror());
		return NULL;
	}

	client = client_find(NULL, &ipaddr);
	if (!client) {
		cprintf(listener, "ERROR: No such client\n");
		return NULL;
	}

	return client;
}


static int command_show_client_config(rad_listen_t *listener, int argc, char *argv[])
{
	RADCLIENT *client;
	FILE *fp;

	client = get_client(listener, argc, argv);
	if (!client) {
		return 0;
	}

	if (!client->cs) return 1;

	fp = fdopen(dup(listener->fd), "a");
	if (!fp) {
		cprintf(listener, "ERROR: Can't dup %s\n", strerror(errno));
		return 0;
	}

	cf_section2file(fp, client->cs);
	fclose(fp);

	return 1;
}

#ifdef WITH_PROXY
static home_server *get_home_server(rad_listen_t *listener, int argc, char *argv[])
{
	home_server *home;
	int port;
	fr_ipaddr_t ipaddr;

	if (argc < 2) {
		cprintf(listener, "ERROR: Must specify <ipaddr> <port>\n");
		return NULL;
	}

	if (ip_hton(argv[0], AF_UNSPEC, &ipaddr) < 0) {
		cprintf(listener, "ERROR: Failed parsing IP address; %s\n",
			fr_strerror());
		return NULL;
	}

	port = atoi(argv[1]);

	home = home_server_find(&ipaddr, port);
	if (!home) {
		cprintf(listener, "ERROR: No such home server\n");
		return NULL;
	}

	return home;
}

static int command_show_home_server_config(rad_listen_t *listener, int argc, char *argv[])
{
	home_server *home;
	FILE *fp;

	home = get_home_server(listener, argc, argv);
	if (!home) {
		return 0;
	}

	if (!home->cs) return 1;

	fp = fdopen(dup(listener->fd), "a");
	if (!fp) {
		cprintf(listener, "ERROR: Can't dup %s\n", strerror(errno));
		return 0;
	}

	cf_section2file(fp, home->cs);
	fclose(fp);

	return 1;
}

extern void revive_home_server(void *ctx);
extern void mark_home_server_dead(home_server *home, struct timeval *when);

static int command_set_home_server_state(rad_listen_t *listener, int argc, char *argv[])
{
	home_server *home;

	if (argc < 3) {
		cprintf(listener, "ERROR: Must specify <ipaddr> <port> <state>\n");
		return 0;
	}

	home = get_home_server(listener, argc, argv);
	if (!home) {
		return 0;
	}

	if (strcmp(argv[2], "alive") == 0) {
		revive_home_server(home);

	} else if (strcmp(argv[2], "dead") == 0) {
		struct timeval now;

		gettimeofday(&now, NULL); /* we do this WAY too ofetn */
		mark_home_server_dead(home, &now);

	} else {
		cprintf(listener, "ERROR: Unknown state \"%s\"\n", argv[2]);
		return 0;
	}

	return 1;
}

static int command_show_home_server_state(rad_listen_t *listener, int argc, char *argv[])
{
	home_server *home;

	home = get_home_server(listener, argc, argv);
	if (!home) {
		return 0;
	}

	switch (home->state) {
	case HOME_STATE_ALIVE:
		cprintf(listener, "alive\n");
		break;

	case HOME_STATE_IS_DEAD:
		cprintf(listener, "dead\n");
		break;

	case HOME_STATE_ZOMBIE:
		cprintf(listener, "zombie\n");
		break;

	default:
		cprintf(listener, "unknown\n");
		break;
	}
	
	return 1;
}
#endif

/*
 *	For encode/decode stuff
 */
static int null_socket_dencode(UNUSED rad_listen_t *listener, UNUSED REQUEST *request)
{
	return 0;
}

static int null_socket_send(UNUSED rad_listen_t *listener, REQUEST *request)
{
	char *output_file;
	FILE *fp;
	VALUE_PAIR *vp;

	output_file = request_data_reference(request, null_socket_send, 0);
	if (!output_file) {
		radlog(L_ERR, "WARNING: No output file for injected packet %d",
		       request->number);
		return 0;
	}

	fp = fopen(output_file, "w");
	if (!fp) {
		radlog(L_ERR, "Failed to send injected file to %s: %s",
		       output_file, strerror(errno));
		return 0;
	}

	if (request->reply->code != 0) {
		const char *what = "reply";
		char buffer[1024];

		if (request->reply->code < FR_MAX_PACKET_CODE) {
			what = fr_packet_codes[request->reply->code];
		}

		fprintf(fp, "%s\n", what);

		if (debug_flag) {
			request->radlog(L_DBG, 0, request,
					"Injected %s packet to host %s port 0 code=%d, id=%d",
					what,
					inet_ntop(request->reply->src_ipaddr.af,
						  &request->reply->src_ipaddr.ipaddr,
						  buffer, sizeof(buffer)),
					request->reply->code, request->reply->id);
		}

		for (vp = request->reply->vps; vp != NULL; vp = vp->next) {
			vp_prints(buffer, sizeof(buffer), vp);
			fprintf(fp, "%s\n", buffer);
			if (debug_flag) {
				request->radlog(L_DBG, 0, request, "\t%s",
						buffer);
			}
		}
	}
	fclose(fp);

	return 0;
}

static int command_inject_to(rad_listen_t *listener, int argc, char *argv[])
{
	int port;
	RAD_LISTEN_TYPE type;
	fr_command_socket_t *sock = listener->data;
	fr_ipaddr_t ipaddr;
	rad_listen_t *found = NULL;

	if (argc < 1) {
		cprintf(listener, "ERROR: Must specify [auth/acct]\n");
		return 0;
	}

	if (strcmp(argv[0], "auth") == 0) {
		type = RAD_LISTEN_AUTH;

	} else if (strcmp(argv[0], "acct") == 0) {
#ifdef WITH_ACCOUNTING
		type = RAD_LISTEN_ACCT;
#else
		cprintf(listener, "ERROR: This server was built without accounting support.\n");
		return 0;
#endif

	} else {
		cprintf(listener, "ERROR: Unknown socket type\n");
		return 0;
	}

	if (argc < 3) {
		cprintf(listener, "ERROR: No <ipaddr> <port> was given\n");
		return 0;
	}

	/*
	 *	FIXME:  Look for optional arg 4, and bind interface.
	 */

	if (ip_hton(argv[1], AF_UNSPEC, &ipaddr) < 0) {
		cprintf(listener, "ERROR: Failed parsing IP address; %s\n",
			fr_strerror());
		return 0;
	}
	port = atoi(argv[2]);

	found = listener_find_byipaddr(&ipaddr, port);
	if (!found) {
		cprintf(listener, "ERROR: Could not find matching listener\n");
		return 0;
	}

	sock->inject_listener = found;
	sock->dst_ipaddr = ipaddr;
	sock->dst_port = port;

	return 1;
}

static int command_inject_from(rad_listen_t *listener, int argc, char *argv[])
{
	RADCLIENT *client;
	fr_command_socket_t *sock = listener->data;

	if (argc < 1) {
		cprintf(listener, "ERROR: No <ipaddr> was given\n");
		return 0;
	}

	if (!sock->inject_listener) {
		cprintf(listener, "ERROR: You must specify \"inject to\" before using \"inject from\"\n");
		return 0;
	}

	sock->src_ipaddr.af = AF_UNSPEC;
	if (ip_hton(argv[0], AF_UNSPEC, &sock->src_ipaddr) < 0) {
		cprintf(listener, "ERROR: Failed parsing IP address; %s\n",
			fr_strerror());
		return 0;
	}

	client = client_listener_find(sock->inject_listener, &sock->src_ipaddr,
				      0);
	if (!client) {
		cprintf(listener, "ERROR: No such client %s\n", argv[0]);
		return 0;
	}
	sock->inject_client = client;

	return 1;
}

static int command_inject_file(rad_listen_t *listener, int argc, char *argv[])
{
	static int inject_id = 0;
	int filedone;
	fr_command_socket_t *sock = listener->data;
	rad_listen_t *fake;
	REQUEST *request = NULL;
	RADIUS_PACKET *packet;
	VALUE_PAIR *vp;
	FILE *fp;
	RAD_REQUEST_FUNP fun = NULL;
	char buffer[2048];

	if (argc < 2) {
		cprintf(listener, "ERROR: You must specify <input-file> <output-file>\n");
		return 0;
	}

	/*
	 *	Output files always go to the logging directory.
	 */
	snprintf(buffer, sizeof(buffer), "%s/%s", radlog_dir, argv[1]);

	fp = fopen(argv[0], "r");
	if (!fp ) {
		cprintf(listener, "ERROR: Failed opening %s: %s\n",
			argv[0], strerror(errno));
		return 0;
	}

	vp = readvp2(fp, &filedone, "");
	fclose(fp);
	if (!vp) {
		cprintf(listener, "ERROR: Failed reading attributes from %s: %s\n",
			argv[0], fr_strerror());
		return 0;
	}

	fake = rad_malloc(sizeof(*fake));
	memcpy(fake, sock->inject_listener, sizeof(*fake));

	/*
	 *	Re-write the IO for the listener.
	 */
	fake->encode = null_socket_dencode;
	fake->decode = null_socket_dencode;
	fake->send = null_socket_send;

	packet = rad_alloc(0);
	packet->src_ipaddr = sock->src_ipaddr;
	packet->src_port = 0;

	packet->dst_ipaddr = sock->dst_ipaddr;
	packet->dst_port = sock->dst_port;
	packet->vps = vp;
	packet->id = inject_id++;

	if (fake->type == RAD_LISTEN_AUTH) {
		packet->code = PW_AUTHENTICATION_REQUEST;
		fun = rad_authenticate;

	} else {
#ifdef WITH_ACCOUNTING
		packet->code = PW_ACCOUNTING_REQUEST;
		fun = rad_accounting;
#else
		cprintf(listener, "ERROR: This server was built without accounting support.\n");
		rad_free(&packet);
		free(fake);
		return 0;
#endif
	}

	if (!received_request(fake, packet, &request, sock->inject_client)) {
		cprintf(listener, "ERROR: Failed to inject request.  See log file for details\n");
		rad_free(&packet);
		free(fake);
		return 0;
	}

	/*
	 *	Remember what the output file is, and remember to
	 *	delete the fake listener when done.
	 */
	request_data_add(request, null_socket_send, 0, strdup(buffer), free);
	request_data_add(request, null_socket_send, 1, fake, free);

	if (debug_flag) {
		request->radlog(L_DBG, 0, request,
				"Injected %s packet from host %s port 0 code=%d, id=%d",
				fr_packet_codes[packet->code],
				inet_ntop(packet->src_ipaddr.af,
					  &packet->src_ipaddr.ipaddr,
					  buffer, sizeof(buffer)),
				packet->code, packet->id);
		
		for (vp = packet->vps; vp != NULL; vp = vp->next) {
			vp_prints(buffer, sizeof(buffer), vp);
			request->radlog(L_DBG, 0, request, "\t%s", buffer);
		}
	}

	/*
	 *	And go process it.
	 */
	thread_pool_addrequest(request, fun);

	return 1;
}


static fr_command_table_t command_table_inject[] = {
	{ "to", FR_WRITE,
	  "inject to <ipaddr> <port> - Inject packets to the destination IP and port.",
	  command_inject_to, NULL },

	{ "from", FR_WRITE,
	  "inject from <ipaddr> - Inject packets as if they came from <ipaddr>",
	  command_inject_from, NULL },

	{ "file", FR_WRITE,
	  "inject file <input-file> <output-file> - Inject packet from input-file>, with results sent to <output-file>",
	  command_inject_file, NULL },

	{ NULL, 0, NULL, NULL, NULL }
};

static fr_command_table_t command_table_debug[] = {
	{ "condition", FR_WRITE,
	  "debug condition [condition] - Enable debugging for requests matching [condition]",
	  command_debug_condition, NULL },

	{ "level", FR_WRITE,
	  "debug level <number> - Set debug level to <number>.  Higher is more debugging.",
	  command_debug_level, NULL },

	{ "file", FR_WRITE,
	  "debug file [filename] - Send all debugging output to [filename]",
	  command_debug_file, NULL },

	{ NULL, 0, NULL, NULL, NULL }
};

static fr_command_table_t command_table_show_debug[] = {
	{ "condition", FR_READ,
	  "show debug condition - Shows current debugging condition.",
	  command_show_debug_condition, NULL },

	{ "level", FR_READ,
	  "show debug level - Shows current debugging level.",
	  command_show_debug_level, NULL },

	{ "file", FR_READ,
	  "show debug file - Shows current debugging file.",
	  command_show_debug_file, NULL },

	{ NULL, 0, NULL, NULL, NULL }
};

static fr_command_table_t command_table_show_module[] = {
	{ "config", FR_READ,
	  "show module config <module> - show configuration for given module",
	  command_show_module_config, NULL },
	{ "flags", FR_READ,
	  "show module flags <module> - show other module properties",
	  command_show_module_flags, NULL },
	{ "list", FR_READ,
	  "show module list - shows list of loaded modules",
	  command_show_modules, NULL },
	{ "methods", FR_READ,
	  "show module methods <module> - show sections where <module> may be used",
	  command_show_module_methods, NULL },

	{ NULL, 0, NULL, NULL, NULL }
};

static fr_command_table_t command_table_show_client[] = {
	{ "config", FR_READ,
	  "show client config <ipaddr> - show configuration for given client",
	  command_show_client_config, NULL },
	{ "list", FR_READ,
	  "show client list - shows list of global clients",
	  command_show_clients, NULL },

	{ NULL, 0, NULL, NULL, NULL }
};

#ifdef WITH_PROXY
static fr_command_table_t command_table_show_home[] = {
	{ "config", FR_READ,
	  "show home_server config <ipaddr> <port> - show configuration for given home server",
	  command_show_home_server_config, NULL },
	{ "list", FR_READ,
	  "show home_server list - shows list of home servers",
	  command_show_home_servers, NULL },
	{ "state", FR_READ,
	  "show home_server state <ipaddr> <port> - shows state of given home server",
	  command_show_home_server_state, NULL },

	{ NULL, 0, NULL, NULL, NULL }
};
#endif


static fr_command_table_t command_table_show[] = {
	{ "client", FR_READ,
	  "show client <command> - do sub-command of client",
	  NULL, command_table_show_client },
	{ "debug", FR_READ,
	  "show debug <command> - show debug properties",
	  NULL, command_table_show_debug },
#ifdef WITH_PROXY
	{ "home_server", FR_READ,
	  "show home_server <command> - do sub-command of home_server",
	  NULL, command_table_show_home },
#endif
	{ "module", FR_READ,
	  "show module <command> - do sub-command of module",
	  NULL, command_table_show_module },
	{ "uptime", FR_READ,
	  "show uptime - shows time at which server started",
	  command_uptime, NULL },
	{ "version", FR_READ,
	  "show version - Prints version of the running server",
	  command_show_version, NULL },
	{ "xml", FR_READ,
	  "show xml <reference> - Prints out configuration as XML",
	  command_show_xml, NULL },
	{ NULL, 0, NULL, NULL, NULL }
};


static int command_set_module_config(rad_listen_t *listener, int argc, char *argv[])
{
	int i, rcode;
	CONF_PAIR *cp;
	CONF_SECTION *cs;
	module_instance_t *mi;
	const CONF_PARSER *variables;
	void *data;

	if (argc < 3) {
		cprintf(listener, "ERROR: No module name or variable was given\n");
		return 0;
	}

	cs = cf_section_find("modules");
	if (!cs) return 0;

	mi = find_module_instance(cs, argv[0], 0);
	if (!mi) {
		cprintf(listener, "ERROR: No such module \"%s\"\n", argv[0]);
		return 0;
	}

	if ((mi->entry->module->type & RLM_TYPE_HUP_SAFE) == 0) {
		cprintf(listener, "ERROR: Cannot change configuration of module as it is cannot be HUP'd.\n");
		return 0;
	}

	variables = cf_section_parse_table(mi->cs);
	if (!variables) {
		cprintf(listener, "ERROR: Cannot find configuration for module\n");
		return 0;
	}

	rcode = -1;
	for (i = 0; variables[i].name != NULL; i++) {
		/*
		 *	FIXME: Recurse into sub-types somehow...
		 */
		if (variables[i].type == PW_TYPE_SUBSECTION) continue;

		if (strcmp(variables[i].name, argv[1]) == 0) {
			rcode = i;
			break;
		}
	}

	if (rcode < 0) {
		cprintf(listener, "ERROR: No such variable \"%s\"\n", argv[1]);
		return 0;
	}

	i = rcode;		/* just to be safe */

	/*
	 *	It's not part of the dynamic configuration.  The module
	 *	needs to re-parse && validate things.
	 */
	if (variables[i].data) {
		cprintf(listener, "ERROR: Variable cannot be dynamically updated\n");
		return 0;
	}

	data = ((char *) mi->insthandle) + variables[i].offset;

	cp = cf_pair_find(mi->cs, argv[1]);
	if (!cp) return 0;

	/*
	 *	Replace the OLD value in the configuration file with
	 *	the NEW value.
	 *
	 *	FIXME: Parse argv[2] depending on it's data type!
	 *	If it's a string, look for leading single/double quotes,
	 *	end then call tokenize functions???
	 */
	cf_pair_replace(mi->cs, cp, argv[2]);

	rcode = cf_item_parse(mi->cs, argv[1], variables[i].type,
			      data, argv[2]);
	if (rcode < 0) {
		cprintf(listener, "ERROR: Failed to parse value\n");
		return 0;
	}

	return 1;		/* success */
}

static int command_set_module_status(rad_listen_t *listener, int argc, char *argv[])
{
	CONF_SECTION *cs;
	module_instance_t *mi;

	if (argc < 2) {
		cprintf(listener, "ERROR: No module name or status was given\n");
		return 0;
	}

	cs = cf_section_find("modules");
	if (!cs) return 0;

	mi = find_module_instance(cs, argv[0], 0);
	if (!mi) {
		cprintf(listener, "ERROR: No such module \"%s\"\n", argv[0]);
		return 0;
	}


	if (strcmp(argv[1], "alive") == 0) {
		mi->dead = FALSE;

	} else if (strcmp(argv[1], "dead") == 0) {
		mi->dead = TRUE;

	} else {
		cprintf(listener, "ERROR: Unknown status \"%s\"\n", argv[2]);
		return 0;
	}

	return 1;		/* success */
}

static int command_print_stats(rad_listen_t *listener, fr_stats_t *stats,
			       int auth)
{
	cprintf(listener, "\trequests\t%d\n", stats->total_requests);
	cprintf(listener, "\tresponses\t%d\n", stats->total_responses);
	
	if (auth) {
		cprintf(listener, "\taccepts\t\t%d\n",
			stats->total_access_accepts);
		cprintf(listener, "\trejects\t\t%d\n",
			stats->total_access_rejects);
		cprintf(listener, "\tchallenges\t%d\n",
			stats->total_access_challenges);
	}

	cprintf(listener, "\tdup\t\t%d\n", stats->total_dup_requests);
	cprintf(listener, "\tinvalid\t\t%d\n", stats->total_invalid_requests);
	cprintf(listener, "\tmalformed\t%d\n", stats->total_malformed_requests);
	cprintf(listener, "\tbad_signature\t%d\n", stats->total_bad_authenticators);
	cprintf(listener, "\tdropped\t\t%d\n", stats->total_packets_dropped);
	cprintf(listener, "\tunknown_types\t%d\n", stats->total_unknown_types);
	
	return 1;
}


#ifdef WITH_PROXY
static int command_stats_home_server(rad_listen_t *listener, int argc, char *argv[])
{
	home_server *home;

	if (argc == 0) {
		cprintf(listener, "ERROR: Must specify [auth/acct] OR <ipaddr> <port>\n");
		return 0;
	}

	if (argc == 1) {
#ifdef WITH_ACCOUNTING
		if (strcmp(argv[0], "acct") == 0) {
			return command_print_stats(listener,
						   &proxy_acct_stats, 0);
		}
#endif
		if (strcmp(argv[0], "auth") == 0) {
			return command_print_stats(listener,
						   &proxy_auth_stats, 1);
		}

		cprintf(listener, "ERROR: Should specify [auth/acct]\n");
		return 0;
	}

	home = get_home_server(listener, argc, argv);
	if (!home) {
		return 0;
	}

	command_print_stats(listener, &home->stats,
			    (home->type == HOME_TYPE_AUTH));
	cprintf(listener, "\toutstanding\t%d\n", home->currently_outstanding);
	return 1;
}
#endif

static int command_stats_client(rad_listen_t *listener, int argc, char *argv[])
{
	int auth = TRUE;
	RADCLIENT *client;

	if (argc < 1) {
		cprintf(listener, "ERROR: Must specify [auth/acct]\n");
		return 0;
	}

	if (strcmp(argv[0], "auth") == 0) {
		auth = TRUE;

	} else if (strcmp(argv[0], "acct") == 0) {
#ifdef WITH_ACCOUNTING
		auth = FALSE;
#else
		cprintf(listener, "ERROR: This server was built without accounting support.\n");
		return 0;
#endif

	} else {
		cprintf(listener, "ERROR: Unknown statistics type\n");
		return 0;
	}

	/*
	 *	Global results for all client.
	 */
	if (argc == 1) {
#ifdef WITH_ACCOUNTING
		if (!auth) {
			return command_print_stats(listener,
						   &radius_acct_stats, auth);
		}
#endif
		return command_print_stats(listener, &radius_auth_stats, auth);
	}

	client = get_client(listener, argc - 1, argv + 1);
	if (!client) {
		return 0;
	}

#ifdef WITH_ACCOUNTING
	if (!auth) {
		return command_print_stats(listener, client->acct, auth);
	}
#endif

	return command_print_stats(listener, client->auth, auth);
}


static int command_add_client_file(rad_listen_t *listener, int argc, char *argv[])
{
	RADCLIENT *c;

	if (argc < 1) {
		cprintf(listener, "ERROR: <file> is required\n");
		return 0;
	}

	/*
	 *	Read the file and generate the client.
	 */
	c = client_read(argv[0], FALSE, FALSE);
	if (!c) {
		cprintf(listener, "ERROR: Unknown error reading client file.\n");
		return 0;
	}

	if (!client_add(NULL, c)) {
		cprintf(listener, "ERROR: Unknown error inserting new client.\n");
		client_free(c);
		return 0;
	}

	return 1;
}


static fr_command_table_t command_table_add_client[] = {
	{ "file", FR_WRITE,
	  "add client file <filename> - Add new client definition from <filename>",
	  command_add_client_file, NULL },

	{ NULL, 0, NULL, NULL, NULL }
};


static fr_command_table_t command_table_add[] = {
	{ "client", FR_WRITE,
	  "add client <command> - Add client configuration commands",
	  NULL, command_table_add_client },

	{ NULL, 0, NULL, NULL, NULL }
};


#ifdef WITH_PROXY
static fr_command_table_t command_table_set_home[] = {
	{ "state", FR_WRITE,
	  "set home_server state <ipaddr> <port> [alive|dead] - set state for given home server",
	  command_set_home_server_state, NULL },

	{ NULL, 0, NULL, NULL, NULL }
};
#endif

static fr_command_table_t command_table_set_module[] = {
	{ "config", FR_WRITE,
	  "set module config <module> variable value - set configuration for <module>",
	  command_set_module_config, NULL },

	{ "status", FR_WRITE,
	  "set module status [alive|dead] - set the module to be alive or dead (always return \"fail\")",
	  command_set_module_status, NULL },

	{ NULL, 0, NULL, NULL, NULL }
};


static fr_command_table_t command_table_set[] = {
	{ "module", FR_WRITE,
	  "set module <command> - set module commands",
	  NULL, command_table_set_module },
#ifdef WITH_PROXY
	{ "home_server", FR_WRITE, 
	  "set home_server <command> - set home server commands",
	  NULL, command_table_set_home },
#endif

	{ NULL, 0, NULL, NULL, NULL }
};


static fr_command_table_t command_table_stats[] = {
	{ "client", FR_READ,
	  "stats client [auth/acct] <ipaddr> - show statistics for given client, or for all clients (auth or acct)",
	  command_stats_client, NULL },
#ifdef WITH_PROXY
	{ "home_server", FR_READ,
	  "stats home_server [<ipaddr>/auth/acct] <port> - show statistics for given home server (ipaddr and port), or for all home servers (auth or acct)",
	  command_stats_home_server, NULL },
#endif

	{ NULL, 0, NULL, NULL, NULL }
};

static fr_command_table_t command_table[] = {
	{ "add", FR_WRITE, NULL, NULL, command_table_add },
	{ "debug", FR_WRITE,
	  "debug <command> - debugging commands",
	  NULL, command_table_debug },
	{ "hup", FR_WRITE,
	  "hup [module] - sends a HUP signal to the server, or optionally to one module",
	  command_hup, NULL },
	{ "inject", FR_WRITE,
	  "inject <command> - commands to inject packets into a running server",
	  NULL, command_table_inject },
	{ "reconnect", FR_READ,
	  "reconnect - reconnect to a running server",
	  NULL, NULL },		/* just here for "help" */
	{ "terminate", FR_WRITE,
	  "terminate - terminates the server, and cause it to exit",
	  command_terminate, NULL },
	{ "set", FR_WRITE, NULL, NULL, command_table_set },
	{ "show",  FR_READ, NULL, NULL, command_table_show },
	{ "stats",  FR_READ, NULL, NULL, command_table_stats },

	{ NULL, 0, NULL, NULL, NULL }
};


/*
 *	Parse the unix domain sockets.
 *
 *	FIXME: TCP + SSL, after RadSec is in.
 */
static int command_socket_parse(CONF_SECTION *cs, rad_listen_t *this)
{
	fr_command_socket_t *sock;

	sock = this->data;

	if (cf_section_parse(cs, sock, command_config) < 0) {
		return -1;
	}

#if defined(HAVE_GETPEEREID) || defined (SO_PEERCRED)
	if (sock->uid_name) {
		struct passwd *pw;
		
		pw = getpwnam(sock->uid_name);
		if (!pw) {
			radlog(L_ERR, "Failed getting uid for %s: %s",
			       sock->uid_name, strerror(errno));
			return -1;
		}

		sock->uid = pw->pw_uid;
	}

	if (sock->gid_name) {
		struct group *gr;

		gr = getgrnam(sock->gid_name);
		if (!gr) {
			radlog(L_ERR, "Failed getting gid for %s: %s",
			       sock->gid_name, strerror(errno));
			return -1;
		}
		sock->gid = gr->gr_gid; 
	}

#else  /* can't get uid or gid of connecting user */

	if (sock->uid_name || sock->gid_name) {
		radlog(L_ERR, "System does not support uid or gid authentication for sockets");
		return -1;
	}

#endif

	if (!sock->mode_name) {
		sock->mode = FR_READ;
	} else {
		sock->mode = fr_str2int(mode_names, sock->mode_name, 0);
		if (!sock->mode) {
			radlog(L_ERR, "Invalid mode name \"%s\"",
			       sock->mode_name);
			return -1;
		}
	}

	/*
	 *	FIXME: check for absolute pathnames?
	 *	check for uid/gid on the other end...	 
	 */

	this->fd = fr_server_domain_socket(sock->path);
	if (this->fd < 0) {
		return -1;
	}

	return 0;
}

static int command_socket_print(rad_listen_t *this, char *buffer, size_t bufsize)
{
	fr_command_socket_t *sock = this->data;

	snprintf(buffer, bufsize, "command file %s", sock->path);
	return 1;
}


/*
 *	String split routine.  Splits an input string IN PLACE
 *	into pieces, based on spaces.
 */
static int str2argv(char *str, char **argv, int max_argc)
{
	int argc = 0;
	size_t len;
	char buffer[1024];

	while (*str) {
		if (argc >= max_argc) return argc;

		/*
		 *	Chop out comments early.
		 */
		if (*str == '#') {
			*str = '\0';
			break;
		}

		while ((*str == ' ') ||
		       (*str == '\t') ||
		       (*str == '\r') ||
		       (*str == '\n')) *(str++) = '\0';

		if (!*str) return argc;

		if ((*str == '\'') || (*str == '"')) {
			char *p = str;
			FR_TOKEN token;

			token = gettoken((const char **) &p, buffer,
					 sizeof(buffer));
			if ((token != T_SINGLE_QUOTED_STRING) &&
			    (token != T_DOUBLE_QUOTED_STRING)) {
				return -1;
			}

			len = strlen(buffer);
			if (len >= (size_t) (p - str)) {
				return -1;
			}

			memcpy(str, buffer, len + 1);
			argv[argc] = str;
			str = p;

		} else {
			argv[argc] = str;
		}
		argc++;

		while (*str &&
		       (*str != ' ') &&
		       (*str != '\t') &&
		       (*str != '\r') &&
		       (*str != '\n')) str++;
	}

	return argc;
}

static void print_help(rad_listen_t *listener,
		       fr_command_table_t *table, int recursive)
{
	int i;
	
	for (i = 0; table[i].command != NULL; i++) {
		if (table[i].help) {
			cprintf(listener, "%s\n",
				table[i].help);
		} else {
			cprintf(listener, "%s <command> - do sub-command of %s\n",
				table[i].command, table[i].command);
		}

		if (recursive && table[i].table) {
			print_help(listener, table[i].table, recursive);
		}
	}
}

#define MAX_ARGV (16)

/*
 *	Check if an incoming request is "ok"
 *
 *	It takes packets, not requests.  It sees if the packet looks
 *	OK.  If so, it does a number of sanity checks on it.
 */
static int command_domain_recv(rad_listen_t *listener,
			       UNUSED RAD_REQUEST_FUNP *pfun,
			       UNUSED REQUEST **prequest)
{
	int i, rcode;
	ssize_t len;
	int argc;
	char *my_argv[MAX_ARGV], **argv;
	fr_command_table_t *table;
	fr_command_socket_t *co = listener->data;

	*pfun = NULL;
	*prequest = NULL;

	do {
		ssize_t c;
		char *p;

		len = recv(listener->fd, co->buffer + co->offset,
			   sizeof(co->buffer) - co->offset - 1, 0);
		if (len == 0) goto close_socket; /* clean close */

		if (len < 0) {
			if ((errno == EAGAIN) || (errno == EINTR)) {
				return 0;
			}
			goto close_socket;
		}

		/*
		 *	CTRL-D
		 */
		if ((co->offset == 0) && (co->buffer[0] == 0x04)) {
		close_socket:
			command_close_socket(listener);
			return 0;
		}

		/*
		 *	See if there are multiple lines in the buffer.
		 */
		p = co->buffer + co->offset;
		rcode = 0;
		p[len] = '\0';
		for (c = 0; c < len; c++) {
			if ((*p == '\r') || (*p == '\n')) {
				rcode = 1;
				*p = '\0';

				/*
				 *	FIXME: do real buffering...
				 *	handling of CTRL-C, etc.
				 */

			} else if (rcode) {
				/*
				 *	\r \n followed by ASCII...
				 */
				break;
			}

			p++;
		}

		co->offset += len;

		/*
		 *	Saw CR/LF.  Set next element, and exit.
		 */
		if (rcode) {
			co->next = p - co->buffer;
			break;
		}

		if (co->offset >= (ssize_t) (sizeof(co->buffer) - 1)) {
			radlog(L_ERR, "Line too long!");
			goto close_socket;
		}

		co->offset++;
	} while (1);

	argc = str2argv(co->buffer, my_argv, MAX_ARGV);
	if (argc == 0) goto do_next; /* empty strings are OK */

	if (argc < 0) {
		cprintf(listener, "ERROR: Failed parsing command.\n");
		goto do_next;
	}

	argv = my_argv;

	for (len = 0; len <= co->offset; len++) {
		if (co->buffer[len] < 0x20) {
			co->buffer[len] = '\0';
			break;
		}
	}

	/*
	 *	Hard-code exit && quit.
	 */
	if ((strcmp(argv[0], "exit") == 0) ||
	    (strcmp(argv[0], "quit") == 0)) goto close_socket;

#if 0
	if (!co->user[0]) {
		if (strcmp(argv[0], "login") != 0) {
			cprintf(listener, "ERROR: Login required\n");
			goto do_next;
		}

		if (argc < 3) {
			cprintf(listener, "ERROR: login <user> <password>\n");
			goto do_next;
		}

		/*
		 *	FIXME: Generate && process fake RADIUS request.
		 */
		if ((strcmp(argv[1], "root") == 0) &&
		    (strcmp(argv[2], "password") == 0)) {
			strlcpy(co->user, argv[1], sizeof(co->user));
			goto do_next;
		}

		cprintf(listener, "ERROR: Login incorrect\n");
		goto do_next;
	}
#endif

	table = command_table;
 retry:
	len = 0;
	for (i = 0; table[i].command != NULL; i++) {
		if (strcmp(table[i].command, argv[0]) == 0) {
			/*
			 *	Check permissions.
			 */
			if (((co->mode & FR_WRITE) == 0) &&
			    ((table[i].mode & FR_WRITE) != 0)) {
				cprintf(listener, "ERROR: You do not have write permission.  See \"mode = rw\" in %s\n", co->path);
				goto do_next;
			}

			if (table[i].table) {
				/*
				 *	This is the last argument, but
				 *	there's a sub-table.  Print help.
				 *	
				 */
				if (argc == 1) {
					table = table[i].table;
					goto do_help;
				}

				argc--;
				argv++;
				table = table[i].table;
				goto retry;
			}

			if ((argc == 2) && (strcmp(argv[1], "?") == 0)) goto do_help;

			if (!table[i].func) {
				cprintf(listener, "ERROR: Invalid command\n");
				goto do_next;
			}

			len = 1;
			rcode = table[i].func(listener,
					      argc - 1, argv + 1);
			break;
		}
	}

	/*
	 *	No such command
	 */
	if (!len) {
		if ((strcmp(argv[0], "help") == 0) ||
		    (strcmp(argv[0], "?") == 0)) {
			int recursive;

		do_help:
			if ((argc > 1) && (strcmp(argv[1], "-r") == 0)) {
				recursive = TRUE;
			} else {
				recursive = FALSE;
			}

			print_help(listener, table, recursive);
			goto do_next;
		}

		cprintf(listener, "ERROR: Unknown command \"%s\"\n",
			argv[0]);
	}

 do_next:
	cprintf(listener, "radmin> ");

	if (co->next <= co->offset) {
		co->offset = 0;
	} else {
		memmove(co->buffer, co->buffer + co->next,
			co->offset - co->next);
		co->offset -= co->next;
	}

	return 0;
}


static int command_domain_accept(rad_listen_t *listener,
				 UNUSED RAD_REQUEST_FUNP *pfun,
				 UNUSED REQUEST **prequest)
{
	int newfd;
	uint32_t magic;
	rad_listen_t *this;
	socklen_t salen;
	struct sockaddr_storage src;
	fr_command_socket_t *sock = listener->data;
	
	salen = sizeof(src);

	DEBUG2(" ... new connection request on command socket.");

	*pfun = NULL;
	*prequest = NULL;
	
	newfd = accept(listener->fd, (struct sockaddr *) &src, &salen);
	if (newfd < 0) {
		/*
		 *	Non-blocking sockets must handle this.
		 */
		if (errno == EWOULDBLOCK) {
			return 0;
		}

		DEBUG2(" ... failed to accept connection.");
		return 0;
	}

	/*
	 *	Perform user authentication.
	 */
	if (sock->uid_name || sock->gid_name) {
		uid_t uid;
		gid_t gid;

		if (getpeereid(listener->fd, &uid, &gid) < 0) {
			radlog(L_ERR, "Failed getting peer credentials for %s: %s",
			       sock->path, strerror(errno));
			close(newfd);
			return 0;
		}

		if (sock->uid_name && (sock->uid != uid)) {
			radlog(L_ERR, "Unauthorized connection to %s from uid %ld",
			       sock->path, (long int) uid);
			close(newfd);
			return 0;
		}

		if (sock->gid_name && (sock->gid != gid)) {
			radlog(L_ERR, "Unauthorized connection to %s from gid %ld",
			       sock->path, (long int) gid);
			close(newfd);
			return 0;
		}
	}

	/*
	 *	Write 32-bit magic number && version information.
	 */
	magic = htonl(0xf7eead15);
	if (write(newfd, &magic, 4) < 0) {
		radlog(L_ERR, "Failed writing initial data to socket: %s",
		       strerror(errno));
		close(newfd);
		return 0;
	}
	magic = htonl(1);	/* protocol version */
	if (write(newfd, &magic, 4) < 0) {
		radlog(L_ERR, "Failed writing initial data to socket: %s",
		       strerror(errno));
		close(newfd);
		return 0;
	}


	/*
	 *	Add the new listener.
	 */
	this = listen_alloc(listener->type);
	if (!this) return 0;

	/*
	 *	Copy everything, including the pointer to the socket
	 *	information.
	 */
	sock = this->data;
	memcpy(this, listener, sizeof(*this));
	this->status = RAD_LISTEN_STATUS_INIT;
	this->next = NULL;
	this->data = sock;	/* fix it back */

	sock->offset = 0;
	sock->user[0] = '\0';
	sock->path = ((fr_command_socket_t *) listener->data)->path;
	sock->mode = ((fr_command_socket_t *) listener->data)->mode;

	this->fd = newfd;
	this->recv = command_domain_recv;

	/*
	 *	Tell the event loop that we have a new FD
	 */
	event_new_fd(this);

	return 0;
}


/*
 *	Send an authentication response packet
 */
static int command_domain_send(UNUSED rad_listen_t *listener,
			       UNUSED REQUEST *request)
{
	return 0;
}


static int command_socket_encode(UNUSED rad_listen_t *listener,
				 UNUSED REQUEST *request)
{
	return 0;
}


static int command_socket_decode(UNUSED rad_listen_t *listener,
				 UNUSED REQUEST *request)
{
	return 0;
}

#endif /* WITH_COMMAND_SOCKET */

#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=freeradius2-wpe
PKG_VERSION:=2.1.7
PKG_RELEASE:=7

PKG_SOURCE:=freeradius-wpe-2.1.7.tar.gz
PKG_SOURCE_URL:=http://www.cs.stevens.edu/~bridings/

PKG_BUILD_DIR:=$(BUILD_DIR)/freeradius-server-$(PKG_VERSION)
PKG_FIXUP:=libtool

include $(INCLUDE_DIR)/package.mk

define Package/freeradius2-wpe/Default
  SECTION:=net
  CATEGORY:=Network
  URL:=http://freeradius.org/
endef

define Package/freeradius2-wpe
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=+libltdl +libopenssl +libpthread +libreadline
  TITLE:=A flexible RADIUS server (version 2)
  MENU:=1
endef

define Package/freeradius2-wpe/conffiles
/etc/freeradius2-wpe/clients.conf
/etc/freeradius2-wpe/radiusd.conf
/etc/freeradius2-wpe/sites/default
endef

define Package/freeradius2-wpe-democerts
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=Demo certificates to test the server
endef

define Package/freeradius2-wpe-mod-chap
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=CHAP module
endef

define Package/freeradius2-wpe-mod-chap/conffiles
/etc/freeradius2-wpe/modules/chap
endef

define Package/freeradius2-wpe-mod-detail
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=Detailed accounting module
endef

define Package/freeradius2-wpe-mod-detail/conffiles
/etc/freeradius2-wpe/modules/detail
endef

define Package/freeradius2-wpe-mod-eap
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=Base EAP module
endef

define Package/freeradius2-wpe-mod-eap/conffiles
/etc/freeradius2-wpe/eap.conf
endef

define Package/freeradius2-wpe-mod-eap-gtc
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe-mod-eap
  TITLE:=EAP/GTC module
endef

define Package/freeradius2-wpe-mod-eap-md5
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe-mod-eap
  TITLE:=EAP/MD5 module
endef

define Package/freeradius2-wpe-mod-eap-mschapv2
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe-mod-eap +freeradius2-wpe-mod-mschap
  TITLE:=EAP/MS-CHAPv2 module
endef

define Package/freeradius2-wpe-mod-eap-peap
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe-mod-eap
  TITLE:=EAP/PEAP module
endef

define Package/freeradius2-wpe-mod-eap-tls
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe-mod-eap
  TITLE:=EAP/TLS module
endef

define Package/freeradius2-wpe-mod-eap-ttls
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe-mod-eap-tls
  TITLE:=EAP/TTLS module
endef

define Package/freeradius2-wpe-mod-exec
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=EXEC module
endef

define Package/freeradius2-wpe-mod-exec/conffiles
/etc/freeradius2-wpe/modules/exec
endef

define Package/freeradius2-wpe-mod-expiration
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=Expiration module
endef

define Package/freeradius2-wpe-mod-expiration/conffiles
/etc/freeradius2-wpe/modules/expiration
endef

define Package/freeradius2-wpe-mod-always
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=Always module
endef

define Package/freeradius2-wpe-mod-always/conffiles
/etc/freeradius2-wpe/modules/always
endef

define Package/freeradius2-wpe-mod-expr
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=EXPR module
endef

define Package/freeradius2-wpe-mod-expr/conffiles
/etc/freeradius2-wpe/modules/expr
endef

define Package/freeradius2-wpe-mod-attr-filter
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=ATTR filter module
endef

define Package/freeradius2-wpe-mod-attr-filter/conffiles
/etc/freeradius2-wpe/modules/attr_filter
/etc/freeradius2-wpe/attrs
/etc/freeradius2-wpe/attrs.access_reject
/etc/freeradius2-wpe/attrs.accounting_response
/etc/freeradius2-wpe/attrs.pre-proxy
endef

define Package/freeradius2-wpe-mod-attr-rewrite
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=ATTR rewrite module
endef

define Package/freeradius2-wpe-mod-attr-rewrite/conffiles
/etc/freeradius2-wpe/modules/attr_rewrite
endef

define Package/freeradius2-wpe-mod-files
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=Module using local files for authorization
endef

define Package/freeradius2-wpe-mod-files/conffiles
/etc/freeradius2-wpe/acct_users
/etc/freeradius2-wpe/preproxy_users
/etc/freeradius2-wpe/users
/etc/freeradius2-wpe/modules/files
endef

define Package/freeradius2-wpe-mod-passwd
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=Rlm passwd module
endef

define Package/freeradius2-wpe-mod-passwd/conffiles
/etc/freeradius2-wpe/modules/passwd
endef

define Package/freeradius2-wpe-mod-ldap
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe +PACKAGE_freeradius2-wpe-mod-ldap:libopenldap
  TITLE:=LDAP module
endef

define Package/freeradius2-wpe-mod-ldap/conffiles
/etc/freeradius2-wpe/ldap.attrmap
/etc/freeradius2-wpe/modules/ldap
endef

define Package/freeradius2-wpe-mod-logintime
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=Logintime module
endef

define Package/freeradius2-wpe-mod-logintime/conffiles
/etc/freeradius2-wpe/modules/logintime
endef

define Package/freeradius2-wpe-mod-mschap
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=MS-CHAP and MS-CHAPv2 module
endef

define Package/freeradius2-wpe-mod-mschap/conffiles
/etc/freeradius2-wpe/modules/mschap
endef

define Package/freeradius2-wpe-mod-pap
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=PAP module
endef

define Package/freeradius2-wpe-mod-pap/conffiles
/etc/freeradius2-wpe/modules/pap
endef

define Package/freeradius2-wpe-mod-preprocess
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=Request pre-processing module
endef

define Package/freeradius2-wpe-mod-preprocess/conffiles
/etc/freeradius2-wpe/hints
/etc/freeradius2-wpe/huntgroups
/etc/freeradius2-wpe/modules/preprocess
endef

define Package/freeradius2-wpe-mod-realm
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=Realms handling module
endef

define Package/freeradius2-wpe-mod-realm/conffiles
/etc/freeradius2-wpe/proxy.conf
/etc/freeradius2-wpe/modules/realm
endef

define Package/freeradius2-wpe-mod-sql
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=Base SQL module
endef

define Package/freeradius2-wpe-mod-sql/conffiles
/etc/freeradius2-wpe/sql.conf
endef

define Package/freeradius2-wpe-mod-sql-mysql
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe-mod-sql \
  	+PACKAGE_freeradius2-wpe-mod-sql-mysql:libmysqlclient_r
  TITLE:=MySQL module
endef

define Package/freeradius2-wpe-mod-sql-pgsql
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe-mod-sql \
  	+PACKAGE_freeradius2-wpe-mod-sql-pgsql:libpq
  TITLE:=PostgreSQL module
endef

define Package/freeradius2-wpe-mod-sql-sqlite
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe-mod-sql \
  	+PACKAGE_freeradius2-wpe-mod-sql-sqlite:libsqlite3
  TITLE:=SQLite module
endef

define Package/freeradius2-wpe-mod-sqlcounter
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe-mod-sql
  TITLE:=Generic SQL Counter module
endef

define Package/freeradius2-wpe-mod-radutmp
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=Radius UTMP module
endef

define Package/freeradius2-wpe-mod-radutmp/conffiles
/etc/freeradius2-wpe/modules/radutmp
/etc/freeradius2-wpe/modules/sradutmp
endef

define Package/freeradius2-wpe-utils
  $(call Package/freeradius2-wpe/Default)
  DEPENDS:=freeradius2-wpe
  TITLE:=Misc. client utilities
endef

CONFIGURE_ARGS+= \
	--libdir=/usr/lib/freeradius2-wpe \
	--libdir=$(STAGING_DIR)/host/lib/ \
	--libexecdir=/usr/lib/freeradius2-wpe \
	--enable-shared \
	--disable-static \
	--disable-developer \
	--with-system-libtool \
	--with-system-libltdl \
	--with-threads \
	--with-openssl-includes="$(STAGING_DIR)/usr/include" \
	--with-openssl-libraries="$(STAGING_DIR)/usr/lib" \
	--enable-strict-dependencies \
	--with-raddbdir=/etc/freeradius2-wpe \
	--with-radacctdir=/var/db/radacct \
	--with-logdir=/var/log \
	--without-edir \
	--without-snmp \
	--without-rlm_checkval \
	--without-rlm_dbm \
	--without-rlm_counter \
	--with-rlm_expr \
	--with-rlm_eap \
	--without-rlm_eap_sim \
	--without-rlm_example \
	--without-rlm_ippool \
	--without-rlm_ldap \
	--without-rlm_krb5 \
	--without-rlm_otp \
	--without-rlm_smsotp \
	--without-rlm_pam \
	--without-rlm_perl \
	--without-rlm_python \
	--without-rlm_smb \
	--with-rlm_sql \
	--with-rlm_sqlcounter \
	--without-rlm_sqlhpwippool \
	--without-rlm_sqlippool \
	--without-rlm_sql_db2 \
	--without-rlm_sql_firebird \
	--without-rlm_sql_freetds \
	--without-rlm_sql_iodbc \
	--without-rlm_sql_oracle \
	--without-rlm_sql_sqlite \
	--without-rlm_sql_sybase \
	--without-rlm_sql_unixodbc \
	--without-rlm_sql_log \
	--without-rlm_unix \
	--without-rlm_eap_tnc \
	--without-rlm_eap_ikev2 \
	--without-rlm_opendirectory \
	--without-rlm_wimax \
	--without-rlm_ruby \

PKG_DICTIONARIES:= \
	freeradius freeradius.internal \
	rfc2865 rfc2866 rfc2867 rfc2868 rfc2869 rfc3162 rfc3576 rfc3580 \
	rfc4372 rfc4675 rfc4679 \
	microsoft \
	wispr \

ifneq ($(SDK)$(CONFIG_PACKAGE_freeradius2-wpe-mod-ldap),)
  CONFIGURE_ARGS+= \
		--with-rlm_ldap-include-dir="$(STAGING_DIR)/usr/include" \
		--with-rlm_ldap-lib-dir="$(STAGING_DIR)/usr/lib"
  CONFIGURE_LIBS+= -lcrypto -lssl
else
  CONFIGURE_ARGS+= --without-rlm_ldap
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_freeradius2-wpe-mod-sql-mysql),)
  CONFIGURE_ARGS+= \
		--with-mysql-include-dir="$(STAGING_DIR)/usr/include" \
		--with-mysql-lib-dir="$(STAGING_DIR)/usr/lib/mysql"
  CONFIGURE_LIBS+= -lz
  CONFIGURE_VARS+= ac_cv_lib_mysqlclient_r_mysql_init=yes
else
  CONFIGURE_ARGS+= --without-rlm_sql_mysql
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_freeradius2-wpe-mod-sql-pgsql),)
  CONFIGURE_ARGS+= \
		--with-rlm_sql_postgresql-include-dir="$(STAGING_DIR)/usr/include" \
		--with-rlm_sql_postgresql-lib-dir="$(STAGING_DIR)/usr/lib"
else
  CONFIGURE_ARGS+= --without-rlm_sql_postgresql
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_freeradius2-wpe-mod-sql-sqlite),)
  CONFIGURE_ARGS+= \
	--with-rlm_sql_sqlite \
	--with-experimental-modules \
	--with-sqlite-include-dir="$(STAGING_DIR)/usr/include" \
	--with-sqlite-lib-dir="$(STAGING_DIR)/usr/lib"
else
  CONFIGURE_ARGS+= --without-rlm_sql_sqlite
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_freeradius2-wpe-mod-eap-peap),)
  CONFIGURE_ARGS+= \
  		--with-rlm_eap_peap \
		--with-rlm_eap_peap-include-dir="$(STAGING_DIR)/usr/include" \
		--with-rlm_eap_peap-lib-dir="$(STAGING_DIR)/usr/lib"
  CONFIGURE_LIBS+= -lcrypto -lssl
else
  CONFIGURE_ARGS+= --without-rlm_eap_peap
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_freeradius2-wpe-mod-eap-tls),)
  CONFIGURE_ARGS+= \
  		--with-rlm_eap_tls \
		--with-rlm_eap_tls-include-dir="$(STAGING_DIR)/usr/include" \
		--with-rlm_eap_tls-lib-dir="$(STAGING_DIR)/usr/lib"
  CONFIGURE_LIBS+= -lcrypto -lssl
else
  CONFIGURE_ARGS+= --without-rlm_eap_tls
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_freeradius2-wpe-mod-eap-ttls),)
  CONFIGURE_ARGS+= \
  		--with-rlm_eap_ttls \
		--with-rlm_eap_ttls-include-dir="$(STAGING_DIR)/usr/include" \
		--with-rlm_eap_ttls-lib-dir="$(STAGING_DIR)/usr/lib"
  CONFIGURE_LIBS+= -lcrypto -lssl
else
  CONFIGURE_ARGS+= --without-rlm_eap_ttls
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_freeradius2-wpe-mod-attr-rewrite),)
  CONFIGURE_ARGS+= --with-rlm_attr-rewrite
else
  CONFIGURE_ARGS+= --without-rlm_attr-rewrite
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_freeradius2-wpe-mod-radutmp),)
  CONFIGURE_ARGS+= --with-rlm_radutmp
else
  CONFIGURE_ARGS+= --without-rlm_radutmp
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_freeradius2-wpe-mod-logintime),)
  CONFIGURE_ARGS+= --with-rlm_logintime
else
  CONFIGURE_ARGS+= --without-rlm_logintime
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_freeradius2-wpe-mod-expiration),)
  CONFIGURE_ARGS+= --with-rlm_expiration
else
  CONFIGURE_ARGS+= --without-rlm_expiration
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_freeradius2-wpe-mod-always),)
  CONFIGURE_ARGS+= --with-rlm_always
else
  CONFIGURE_ARGS+= --without-rlm_always
endif

CONFIGURE_VARS+= \
	LDFLAGS="$$$$LDFLAGS" \
	LIBS="$(CONFIGURE_LIBS)" \
	MYSQL_CONFIG="no" \
	ac_cv_lib_readline=no \

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		R="$(PKG_INSTALL_DIR)" \
		INSTALLSTRIP="" \
		all certs install
endef

define Package/freeradius2-wpe/install
	$(INSTALL_DIR) $(1)/etc/freeradius2-wpe
	$(INSTALL_DIR) $(1)/etc/freeradius2-wpe/modules
	$(INSTALL_DIR) $(1)/etc/freeradius2-wpe/sites
	chmod 771 $(1)/etc/freeradius2-wpe
	for f in clients.conf dictionary radiusd.conf policy.conf; do \
		$(CP) $(PKG_INSTALL_DIR)/etc/freeradius2-wpe/$$$${f} $(1)/etc/freeradius2-wpe/ ; \
	done
	$(CP) $(PKG_INSTALL_DIR)/etc/freeradius2-wpe/sites-available/default $(1)/etc/freeradius2-wpe/sites/default
	$(INSTALL_DIR) $(1)/usr/share/freeradius2-wpe
	$(CP) $(PKG_INSTALL_DIR)/usr/share/freeradius/dictionary $(1)/usr/share/freeradius2-wpe/
	$(SED) "s,^\(\$$$$INCLUDE\),#\1,g" $(1)/usr/share/freeradius2-wpe/dictionary
	for f in $(PKG_DICTIONARIES); do \
		$(CP) $(PKG_INSTALL_DIR)/usr/share/freeradius/dictionary.$$$${f} $(1)/usr/share/freeradius2-wpe/ ; \
		$(SED) "s,^#\(\$$$$INCLUDE dictionary\.$$$${f}\),\1,g" $(1)/usr/share/freeradius2-wpe/dictionary ; \
	done
	$(INSTALL_DIR) $(1)/usr/lib/freeradius2-wpe
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/freeradius2-wpe/libfreeradius-radius{,-*}.so $(1)/usr/lib/freeradius2-wpe
	ln -s freeradius2-wpe/libfreeradius-radius-$(PKG_VERSION).so $(1)/usr/lib/
	ln -s freeradius2-wpe/libfreeradius-eap-$(PKG_VERSION).so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/radiusd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/radiusd.init $(1)/etc/init.d/radiusd
endef

define Package/freeradius2-wpe-democerts/install
	$(INSTALL_DIR) $(1)/etc/freeradius2-wpe/certs
	$(CP) \
		$(PKG_BUILD_DIR)/raddb/certs/ca.pem \
		$(PKG_BUILD_DIR)/raddb/certs/dh \
		$(PKG_BUILD_DIR)/raddb/certs/random \
		$(PKG_BUILD_DIR)/raddb/certs/server.pem \
		$(1)/etc/freeradius2-wpe/certs/
endef

define Package/freeradius2-wpe-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	for f in radclient radeapclient radwho; do \
		$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/$$$${f} $(1)/usr/bin/ ; \
	done
endef

define BuildPlugin
  define Package/$(1)/install
	[ -z "$(2)" ] || $(INSTALL_DIR) $$(1)/usr/lib/freeradius2-wpe
	for m in $(2); do \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/freeradius2-wpe/$$$$$$$${m}{,-*}.so $$(1)/usr/lib/freeradius2-wpe ; \
	done
	[ -z "$(3)" ] || $(INSTALL_DIR) $$(1)/etc/freeradius2-wpe
	[ -z "$(4)" ] || $(INSTALL_DIR) $$(1)/etc/freeradius2-wpe/$(4)
	for f in $(3); do \
		$(CP) $(PKG_INSTALL_DIR)/etc/freeradius2-wpe/$$$$$$$${f} $$(1)/etc/freeradius2-wpe/$$$$$$$${f} ; \
	done
  endef

  $$(eval $$(call BuildPackage,$(1)))
endef

$(eval $(call BuildPackage,freeradius2-wpe))
$(eval $(call BuildPackage,freeradius2-wpe-democerts))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-chap,rlm_chap,modules/chap,modules,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-detail,rlm_detail,modules/detail,modules,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-eap,libfreeradius-eap rlm_eap,eap.conf))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-eap-gtc,rlm_eap_gtc,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-eap-md5,rlm_eap_md5,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-eap-mschapv2,rlm_eap_mschapv2,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-eap-peap,rlm_eap_peap,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-eap-tls,rlm_eap_tls,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-eap-ttls,rlm_eap_ttls,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-exec,rlm_exec,modules/exec modules/echo ,modules,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-attr-rewrite,rlm_attr_rewrite,modules/attr_rewrite,modules,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-files,rlm_files,acct_users preproxy_users users modules/files,modules,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-passwd,rlm_passwd,modules/passwd,modules,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-ldap,rlm_ldap,ldap.attrmap modules/ldap,modules,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-mschap,rlm_mschap,modules/mschap,modules,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-pap,rlm_pap,modules/pap,modules,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-preprocess,rlm_preprocess,hints huntgroups modules/preprocess,modules,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-realm,rlm_realm,proxy.conf modules/realm modules/inner-eap,modules,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-sql,rlm_sql,sql.conf,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-sql-mysql,rlm_sql_mysql,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-sql-pgsql,rlm_sql_postgresql,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-sql-sqlite,rlm_sql_sqlite,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-sqlcounter,rlm_sqlcounter,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-radutmp,rlm_radutmp,modules/radutmp modules/sradutmp,modules,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-logintime,rlm_logintime,modules/logintime,modules,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-expr,rlm_expr,modules/expr,modules,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-attr-filter,rlm_attr_filter,modules/attr_filter attrs attrs.access_reject attrs.accounting_response attrs.pre-proxy,modules,,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-expiration,rlm_expiration,modules/expiration,modules,))
$(eval $(call BuildPlugin,freeradius2-wpe-mod-always,rlm_always,modules/always,modules,))
$(eval $(call BuildPackage,freeradius2-wpe-utils))


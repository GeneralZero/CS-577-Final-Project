Install [openwrt image](http://downloads.openwrt.org/attitude_adjustment/12.09-rc2/ar71xx/generic/openwrt-ar71xx-generic-tl-wr703n-v1-squashfs-factory.bin)


Change Configuration to take information from Wireless and pass it on to the wired connection. [Settings](http://wiki.openwrt.org/doc/recipes/bridgedap)

Install Packages

``
aircrack-ng - 1.1-3
aircrack-ptw - 1.0.0-1
base-files - 117-r36088
blkid - 2.21.2-1
block-mount - 0.2.0-9
busybox - 1.19.4-6
curl - 7.29.0-1
dnsmasq - 2.62-2
dropbear - 2011.54-2
firewall - 2-55.1
freeradius2-wpe - 2.2.0-3
freeradius2-wpe-democerts - 2.2.0-3
freeradius2-wpe-mod-always - 2.2.0-3
freeradius2-wpe-mod-attr-filter - 2.2.0-3
freeradius2-wpe-mod-attr-rewrite - 2.2.0-3
freeradius2-wpe-mod-chap - 2.2.0-3
freeradius2-wpe-mod-detail - 2.2.0-3
freeradius2-wpe-mod-eap - 2.2.0-3
freeradius2-wpe-mod-eap-gtc - 2.2.0-3
freeradius2-wpe-mod-eap-md5 - 2.2.0-3
freeradius2-wpe-mod-eap-mschapv2 - 2.2.0-3
freeradius2-wpe-mod-eap-peap - 2.2.0-3
freeradius2-wpe-mod-eap-tls - 2.2.0-3
freeradius2-wpe-mod-eap-ttls - 2.2.0-3
freeradius2-wpe-mod-exec - 2.2.0-3
freeradius2-wpe-mod-expiration - 2.2.0-3
freeradius2-wpe-mod-expr - 2.2.0-3
freeradius2-wpe-mod-files - 2.2.0-3
freeradius2-wpe-mod-logintime - 2.2.0-3
freeradius2-wpe-mod-mschap - 2.2.0-3
freeradius2-wpe-mod-pap - 2.2.0-3
freeradius2-wpe-mod-preprocess - 2.2.0-3
freeradius2-wpe-mod-radutmp - 2.2.0-3
freeradius2-wpe-mod-realm - 2.2.0-3
freeradius2-wpe-utils - 2.2.0-3
git - 1.7.11.2-1
hotplug2 - 1.0-beta-4
iptables - 1.4.10-4
iw - 3.6-1
jshn - 2013-01-29-0bc317aa4d9af44806c28ca286d79a8b5a92b2b8
kernel - 3.3.8-1-d6597ebf6203328d3519ea3c3371a493
kmod-ath - 3.3.8+2012-09-07-3
kmod-ath9k - 3.3.8+2012-09-07-3
kmod-ath9k-common - 3.3.8+2012-09-07-3
kmod-cfg80211 - 3.3.8+2012-09-07-3
kmod-crypto-aes - 3.3.8-1
kmod-crypto-arc4 - 3.3.8-1
kmod-crypto-core - 3.3.8-1
kmod-fs-ext4 - 3.3.8-1
kmod-gpio-button-hotplug - 3.3.8-1
kmod-ipt-conntrack - 3.3.8-1
kmod-ipt-core - 3.3.8-1
kmod-ipt-nat - 3.3.8-1
kmod-ipt-nathelper - 3.3.8-1
kmod-leds-gpio - 3.3.8-1
kmod-ledtrig-default-on - 3.3.8-1
kmod-ledtrig-netdev - 3.3.8-1
kmod-ledtrig-timer - 3.3.8-1
kmod-ledtrig-usbdev - 3.3.8-1
kmod-lib-crc-ccitt - 3.3.8-1
kmod-lib-crc16 - 3.3.8-1
kmod-mac80211 - 3.3.8+2012-09-07-3
kmod-madwifi - 3.3.8+r3314-6
kmod-nls-base - 3.3.8-1
kmod-ppp - 3.3.8-1
kmod-pppoe - 3.3.8-1
kmod-pppox - 3.3.8-1
kmod-scsi-core - 3.3.8-1
kmod-usb-core - 3.3.8-1
kmod-usb-ohci - 3.3.8-1
kmod-usb-storage - 3.3.8-1
kmod-usb2 - 3.3.8-1
kmod-wdt-ath79 - 3.3.8-1
libblkid - 2.21.2-1
libblobmsg-json - 2013-01-29-0bc317aa4d9af44806c28ca286d79a8b5a92b2b8
libc - 0.9.33.2-1
libcurl - 7.29.0-1
libffi - 3.0.10-1
libgcc - 4.6-linaro-1
libgmp - 4.3.1-2
libip4tc - 1.4.10-4
libiwinfo - 36
libiwinfo-lua - 36
libjson - 0.9-2
libltdl - 2.4-1
liblua - 5.1.4-8
libmysqlclient-r - 5.1.53-7
libncurses - 5.7-5
libnl-tiny - 0.1-3
libopenldap - 2.4.23-4
libopenssl - 1.0.1h-1
libpcap - 1.1.1-2
libpq - 9.0.1-3
libpthread - 0.9.33.2-1
libreadline - 5.2-2
libsasl2 - 2.1.23-1
libubox - 2013-01-29-0bc317aa4d9af44806c28ca286d79a8b5a92b2b8
libubus - 2013-01-13-bf566871bd6a633e4504c60c6fc55b2a97305a50
libubus-lua - 2013-01-13-bf566871bd6a633e4504c60c6fc55b2a97305a50
libuci - 2013-01-04.1-1
libuci-lua - 2013-01-04.1-1
libuuid - 2.21.2-1
libxtables - 1.4.10-4
lua - 5.1.4-8
luci - 0.11.1-1
luci-app-firewall - 0.11.1-1
luci-i18n-english - 0.11.1-1
luci-lib-core - 0.11.1-1
luci-lib-ipkg - 0.11.1-1
luci-lib-nixio - 0.11.1-1
luci-lib-sys - 0.11.1-1
luci-lib-web - 0.11.1-1
luci-mod-admin-core - 0.11.1-1
luci-mod-admin-full - 0.11.1-1
luci-proto-core - 0.11.1-1
luci-proto-ppp - 0.11.1-1
luci-sgi-cgi - 0.11.1-1
luci-theme-base - 0.11.1-1
luci-theme-openwrt - 0.11.1-1
mtd - 18.1
nano - 2.2.6-1
netifd - 2013-01-29.2-4bb99d4eb462776336928392010b372236ac3c93
openssl-util - 1.0.1e-1
opkg - 618-3
ppp - 2.4.5-8
ppp-mod-pppoe - 2.4.5-8
pwcrypt - 1.2.2-1
pyopenssl - 0.10-1
python - 2.7.3-1
python-crypto - 2.0.1-1
python-mini - 2.7.3-1
python-openssl - 2.7.3-1
shadow-common - 4.1.5.1-1
shadow-su - 4.1.5.1-1
swap-utils - 2.21.2-1
swconfig - 10
tcpdump - 4.2.1-3
terminfo - 5.7-5
uboot-envtools - 2012.04.01-1
ubus - 2013-01-13-bf566871bd6a633e4504c60c6fc55b2a97305a50
ubusd - 2013-01-13-bf566871bd6a633e4504c60c6fc55b2a97305a50
uci - 2013-01-04.1-1
uclibcxx - 0.2.4-1
uhttpd - 2012-10-30-e57bf6d8bfa465a50eea2c30269acdfe751a46fd
wireless-tools - 29-5
wpad - 20120910-1
zlib - 1.2.7-1
``


Change root to external usb [link](http://en.code-bude.net/2013/02/16/how-to-increase-storage-on-tp-link-wr703n-with-extroot/)

Install Freeradius2-wpe from packages folder.

Install Python and Scappy [link](http://edwardkeeble.com/2014/02/passive-wifi-tracking/)

Install impacket 9.12
`python  setup.py install`

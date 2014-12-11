Install [openwrt image](http://downloads.openwrt.org/attitude_adjustment/12.09-rc2/ar71xx/generic/openwrt-ar71xx-generic-tl-wr703n-v1-squashfs-factory.bin)


Change Configuration to take information from Wireless and pass it on to the wired connection. [Settings](http://wiki.openwrt.org/doc/recipes/bridgedap)

Install Packages

    aircrack-ng
    aircrack-ptw
    base-files
    blkid
    block-mount
    busybox
    curl
    dnsmasq
    dropbear
    firewall
    git
    hotplug2
    iptables
    iw
    jshn
    kernel
    kmod-ath
    kmod-ath9k
    kmod-ath9k-common
    kmod-cfg80211
    kmod-crypto-aes
    kmod-crypto-arc4
    kmod-crypto-core
    kmod-fs-ext4
    kmod-gpio-button-hotplug
    kmod-ipt-conntrack
    kmod-ipt-core
    kmod-ipt-nat
    kmod-ipt-nathelper
    kmod-leds-gpio
    kmod-ledtrig-default-on
    kmod-ledtrig-netdev
    kmod-ledtrig-timer
    kmod-ledtrig-usbdev
    kmod-lib-crc-ccitt
    kmod-lib-crc16
    kmod-mac80211
    kmod-madwifi
    kmod-nls-base
    kmod-ppp
    kmod-pppoe
    kmod-pppox
    kmod-scsi-core
    kmod-usb-core
    kmod-usb-ohci
    kmod-usb-storage
    kmod-usb2
    kmod-wdt-ath79
    libblkid
    libblobmsg-json
    libc
    libcurl
    libffi
    libgcc
    libgmp
    libip4tc
    libiwinfo
    libiwinfo-lua
    libjson
    libltdl
    liblua
    libmysqlclient-r
    libncurses
    libnl-tiny
    libopenldap
    libopenssl
    libpcap
    libpq
    libpthread
    libreadline
    libsasl2
    libubox
    libubus
    libubus-lua
    libuci
    libuci-lua
    libuuid
    libxtables
    lua
    luci
    luci-app-firewall
    luci-i18n-english
    luci-lib-core
    luci-lib-ipkg
    luci-lib-nixio
    luci-lib-sys
    luci-lib-web
    luci-mod-admin-core
    luci-mod-admin-full
    luci-proto-core
    luci-proto-ppp
    luci-sgi-cgi
    luci-theme-base
    luci-theme-openwrt
    mtd
    nano
    netifd
    openssl-util
    opkg
    ppp
    ppp-mod-pppoe
    pwcrypt
    pyopenssl
    python
    python-crypto
    python-mini
    python-openssl
    shadow-common
    shadow-su
    swap-utils
    swconfig
    tcpdump
    terminfo
    uboot-envtools
    ubus
    ubusd
    uci
    uclibcxx
    uhttpd
    wireless-tools
    wpad
    zlib



Change root to external usb [link](http://en.code-bude.net/2013/02/16/how-to-increase-storage-on-tp-link-wr703n-with-extroot/)

Install Freeradius2-wpe* from packages folder.
`opkg install freeradius2-wpe*`

Copy **freeradius-wpe-config.tar.gz** and extract
`tar -xzvf freeradius-wpe-config.tar.gz /`

Install Python and Scappy [link](http://edwardkeeble.com/2014/02/passive-wifi-tracking/)

Install impacket 9.12
`python  setup.py install`

Copy Tools Directory from Repo to router

Create monotor interface through the luci web interface
`http://<router IP>/cgi-bin/luci`

Run **sniff2.py** with monitor interface
`python sniff2.py <Monitor interface ex "wlan0-2">`
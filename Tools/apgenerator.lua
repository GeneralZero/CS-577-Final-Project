require("uci")
require("os")

c = uci.cursor()
name = c:add("wireless", "wifi-iface")

c:set("wireless", name, "device", "radio0")
c:set("wireless", name, "mode", "ap")
c:set("wireless", name, "ssid", arg[1])
c:set("wireless", name, "encryption", argv[2])
c:set("wireless", name, "server", "192.168.1.42")
c:set("wireless", name, "key", "test")

c:save("wireless")
c:commit("wireless")
os.execute("wifi")

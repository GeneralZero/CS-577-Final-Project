require("uci")
require("os")

c = uci.cursor()
name = c:add("wireless", "wifi-iface")

c:set("wireless", name, "device", "radio0")
c:set("wireless", name, "mode", "ap")
c:set("wireless", name, "ssid", arg[1])
c:set("wireless", name, "encryption", "wpa2")
c:set("wireless", name, "server", "127.0.0.1")
c:set("wireless", name, "key", "test")

c:save("wireless")
c:commit("wireless")
os.execute("wifi")

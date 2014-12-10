require("uci")
require("os")

c = uci.cursor()
name = c:add("wireless", "wifi-iface")

c:set("wireless", name, "device", "radio0")
c:set("wireless", name, "mode", "ap")
c:set("wireless", name, "ssid", arg[1])

-- WEP network
if argv[2] == "wep" and argv[3] then
   c:set("wireless", name, "encryption", "wep")
   c:set("wireless", name, "key", argv[3])

-- WPA-Personal network
elseif string.sub(argv[2], 0, 3) == "wpa" and argv[3] then
   c:set("wireless", name, "encryption", "mixed-psk+tkip+ccmp")
   c:set("wireless", name, "key", argv[3])

-- WPA Enterprise network
else
   c:set("wireless", name, "encryption", "mixed-wpa+tkip+ccmp")
   c:set("wireless", name, "key", "test")
   c:set("wireless", name, "server", "192.168.1.42")

end

c:save("wireless")
c:commit("wireless")
os.execute("wifi")

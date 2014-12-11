require("uci")


c = uci.cursor()
name = c:add("wireless", "wifi-iface")

c:set("wireless", name, "device", "radio0")
c:set("wireless", name, "mode", "ap")
c:set("wireless", name, "ssid", arg[1])

-- WEP network
if arg[2] == "wep" and arg[3] then
   c:set("wireless", name, "encryption", "wep")
   c:set("wireless", name, "key", "1")
   c:set("wireless", name, "key1", arg[3])
   
-- WPA-Personal network
elseif string.sub(arg[2], 0, 3) == "wpa" and arg[3] then
   c:set("wireless", name, "encryption", "mixed-psk+tkip+ccmp")
   c:set("wireless", name, "key", arg[3])

-- Open network
elseif arg[2] == "none" then
	c:set("wireless", name, "encryption", "none")

-- WPA Enterprise network
else
   c:set("wireless", name, "encryption", "mixed-wpa+tkip+ccmp")
   c:set("wireless", name, "auth_server", "192.168.1.42")
   c:set("wireless", name, "auth_secret", "test")

end

c:save("wireless")
c:commit("wireless")


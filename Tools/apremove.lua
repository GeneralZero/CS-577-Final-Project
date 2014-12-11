require("uci")

c = uci.cursor()
c:delete("wireless", arg[1])

c:save("wireless")
c:commit("wireless")
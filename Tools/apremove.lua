require("uci")
require("os")

c = uci.cursor()
c:delete("wireless", arg[1])

c:save("wireless")
c:commit("wireless")
os.execute("wifi")

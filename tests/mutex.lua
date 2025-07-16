local llby = require"lullaby"

local th = llby.thread.async(function(res)
  print("start")
  os.execute("sleep 4")
  print("thread")
end)

print("outside")
th:await()

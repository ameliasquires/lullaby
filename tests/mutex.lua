local llby = require"lullaby"


local mutex = llby.thread.mutex()

llby.io.print_meta = 1
llby.io.pprint(mutex)

local th = llby.thread.async(function(res)
  mutex:lock()
  os.execute("sleep 5")
  print("thread")
  mutex:unlock()
end)

os.execute("sleep 1")
mutex:lock()
print("main")
mutex:unlock()
mutex:free()
--
th:await()


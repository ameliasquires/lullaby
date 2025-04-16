a = require "lullaby"

b = coroutine.create(function()
  os.execute("sleep 2")
  print("co")
end)

c = a.thread.async(function(res, req)
  coroutine.resume(b)
end)

os.execute("sleep 0.5")
print("owo")

c:await()

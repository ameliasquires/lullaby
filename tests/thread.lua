llby = require "lullaby"

llby.thread.async(function(res, rej)
  print("hi")
  a = llby.crypto.sha512()
  a:update("hi")
  b = a + "meow"
  print((b + "hi"):final())
  print((a:update("hi")):final())
  print((b + "hi"):final())
end)

os.execute("sleep 1")



llby = require "lullaby"

b = llby.thread.buffer(llby.crypto.md5())

print(b.update(b:get(), "meow"):final())
print(b.update(b:get(), "meow"):final())
b:mod(function(this) return this:update("meow") end)
print(b.update(b:get(), "meow"):final())


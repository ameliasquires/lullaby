local a = 539
local b = "meow meow mrrp!"

local buffer = llby.thread.buffer(a)

local c = buffer:set(b)

return c == a and b == buffer:get()

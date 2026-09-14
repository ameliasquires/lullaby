local a = 539
local b = "meow meow mrrp!"

local atomic = llby.thread.atomic(a)

local c = atomic:set(b)

return c == a and b == atomic:get()

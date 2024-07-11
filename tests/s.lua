local t = function (a) end

require "llib"

local a = llib.crypto.md5()

b = llib.thread.buffer(a)

b:mod(function(e) 
  return e:update("meow")
end)

print(b:get():final())

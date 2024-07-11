local t = function (a) end

require "llib"

local a = llib.crypto.md5()
--llib.io.pprint(_ENV.a)

for i = 1,200 do
  awa = function(a) end
  llib.thread.async(awa):await()
end

if a:final() ~= "d41d8cd98f00b204e9800998ecf8427e" then
  print(a:final())
end

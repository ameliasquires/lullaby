local t = function (a) end

require "llib"

a = llib.crypto.md5()

for i = 1,200 do
  --llib.io.pprint(function (a) end)
  llib.thread.async(function(a) end)--:await()
end

if a:final() ~= "d41d8cd98f00b204e9800998ecf8427e" then
  print(a:final())
end


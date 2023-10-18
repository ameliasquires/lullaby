require "llib"
local a = llib.array

local tab = {}
math.randomseed(os.time())
for i=1,9999999 do
  table.insert(tab,i);--math.random(1,10));-- + math.random(1,999));
end

local l1 = a.shellsort(tab)
print(a.sindex(l1,999));
--print(a.index(l1,1))
for l,i in pairs(l1) do
  --print(i)
end


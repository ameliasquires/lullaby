require "llib"
local a = llib.array

local tab = {}
math.randomseed(os.time())
for i=1,5 do
  table.insert(tab,math.random(1,999));-- + math.random(1,999));
end

local l1 = a.bogosort(tab)

for l,i in pairs(llib.array.reverse(l1)) do
  print(i)
end


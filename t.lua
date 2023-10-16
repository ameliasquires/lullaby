require "llib"
local a = llib.array

local tab = {}
math.randomseed(os.time())
for i=1,19 do
  table.insert(tab,math.random(1,99999));-- + math.random(1,999));
end

print("length of 99999 :\n")
time = os.clock()
local l1 = a.quicksort(tab)
print("quick sort took "..os.clock()-time.."s")
time = os.clock()
local l2 = a.mergesort(tab)
print("merge sort took "..os.clock()-time.."s")
time = os.clock()
local l3 = a.shellsort(tab)
print("shell sort took "..os.clock()-time.."s")
--time = os.clock()
--local l4 = a.bubblesort(tab)
--print("bubble sort took "..os.clock()-time.."s")
time = os.clock()
local l5 = a.heapsort(tab)
print("heap sort took "..os.clock()-time.."s")
time = os.clock()
local l6 = a.countingsort(tab)
print("counting sort took "..os.clock()-time.."s")

for l,i in pairs(l1) do
  print(l1[l].." "..l2[l].." "..l3[l].." "..l5[l].." "..l6[l])
end


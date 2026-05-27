local input = {}
local len = 500
local max = 9999

for i=1,len do
  table.insert(input, math.random(-max, max))
end

local a = llby.table.quicksort(input)
local b = llby.table.bubblesort(input)
local c = llby.table.heapsort(input)
local d = llby.table.shellsort(input)
local e = llby.table.mergesort(input)

return llby.table.equal(a, b) and llby.table.equal(b, c) and
  llby.table.equal(c, d) and llby.table.equal(d, e)

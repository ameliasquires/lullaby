local input = {}
local len = 500
local max = 9999

for i=1,len do
  table.insert(input, math.random(-max, max))
end

local a = llby.table.dup(input)
local b = llby.table.dup(input)
local c = llby.table.dup(input)
local d = llby.table.dup(input)
local e = llby.table.dup(input)

llby.table.quicksort(a)
llby.table.bubblesort(b)
llby.table.heapsort(c)
llby.table.shellsort(d)
llby.table.mergesort(e)

return llby.table.equal(a, b) and llby.table.equal(b, c) and
  llby.table.equal(c, d) and llby.table.equal(d, e)

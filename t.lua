require "llib"
local a = llib.array

local test = {5,"meow",3,2,2,1,8}
test = a.reverse(test)

for i=1,#test do
  print(test[i])
end

print(a.index(test,"meow"))

require "llib"
local a = llib.array

--local test = {5,"meow",3,2,2,1,8}
--test = a.reverse(test)

--for i=1,#test do
--  print(test[i])
--end

--print(a.index(test,"meow"))
local ww = a.to_char_array("meow         awa")--(a.split(llib.io.readfile("c.lua"),"\n"))
for i=1,#ww do
  print(i .. " " .. ww[i])
end

llib.io.debug("meow")
llib.io.log("hmm")
llib.io.warn("nuh uh!")
llib.io.error("void")
print(llib.math.lcm({5,4,33}))

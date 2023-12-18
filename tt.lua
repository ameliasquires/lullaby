require "llib"

local s = llib.array.split("0 5 3 10"," ")
local b = {a=5}
llib.config.set({max_depth=-1});
llib.io.pprint({5,{"what"},{55,55,33,2,5,5,5},{{55,5,8},9},3,{5,{meow=3}},{meow={5,5}}})
--llib.io.pprint(llib.array.len(b))



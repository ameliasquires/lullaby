llby = require("lullaby")
PORT = 5552

local failed = {}
local total = 0

function yay(M)
  print(string.format("\27[32m%s\27[0m passed", M))
end

function nay(M)
  print(string.format("\27[31m%s\27[0m failed", M))
end

local search = ""
if arg[1] ~= nil then
  search = "*" .. arg[1] .. "*"
end

local handle = assert(io.popen("find tests/units/".. search .." -type f"))

for file in handle:lines() do
  total = total + 1
  local f = loadfile(file)()

  if f == true then
    yay(file)
  else
    nay(file)
    table.insert(failed, file)
  end
end

if #failed > 0 then
  print("\n--- failed units (".. #failed .."/".. total ..") ---")
  for _,fail in ipairs(failed) do
    nay(fail)
  end
  print("--- failed units (".. #failed .."/".. total ..") ---")
else
  print("passed all (".. total ..")")
end

handle:close()


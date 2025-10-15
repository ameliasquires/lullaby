local llby = require"lullaby"

local t = llby.thread.async(function(res)
  for i = 1, 50 do
    print(i)
    os.execute("sleep 1")
  end
end)

os.execute("sleep 5")

print("killing")
t:close()
t:await()
print("after kill")

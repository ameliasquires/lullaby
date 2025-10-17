local llby = require"lullaby"

local t, e = llby.thread.async(function(res)
  for i = 1, 50 do
    print(i)
    os.execute("sleep 1")
  end
end)

print(t, e)

print("killing")
t:close()
t:await()
print("after kill")

os.execute("sleep 0.1");

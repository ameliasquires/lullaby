require "llib"

local th = llib.thread.async(function (res)
    _G.llib.io.pprint(res)
    _G.llib.io.pprint({aa=45})
    res:res(_G.llib.crypto.md5())
    print("after")
end)

os.execute("sleep 1")

print("out:")
a = th:res();
print((a + "hi"):final())

while true do end
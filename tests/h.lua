require "llib"

local thread_a = llib.thread.async(function (res)    
    --os.execute("sleep 1")
    --print((_G.ll + "hi"):final())
    res:res(_G.llib.crypto.md5("meow"))
    print("after")
end)

local thread_b = llib.thread.async(function (res)    
    --os.execute("sleep 1")
    --print((_G.ll + "hi"):final())
    res:res(_G.llib.crypto.sha512("meow"))
    print("after")
end)

print(thread_a:await())
print(thread_b:await())
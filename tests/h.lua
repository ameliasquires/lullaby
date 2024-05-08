require "llib"

llib.thread.lock(1)

local thread_a = llib.thread.async(function (res)    
    --os.execute("sleep 1")
    --print((_G.ll + "hi"):final())
    
    res(_G.llib.crypto.md5("meow"))
    print("after")
end)

print(thread_a:await())


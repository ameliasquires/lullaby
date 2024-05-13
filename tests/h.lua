require "llib"

llib.thread.lock(1)
--llib.thread.lock(2)
--llib.thread.unlock(2)

local thread_a = llib.thread.async(function (res)    
    --os.execute("sleep 1")
    --print((_G.ll + "hi"):final())
    print("waiting..")
    llib.thread.lock(1)
    print("signal!")
    res()
    print("after")
end)

os.execute("sleep 1")
print("unlock")
llib.thread.unlock(1)


awa = thread_a:await()

--print((awa + "hi"):final())
os.execute("sleep 1")
thread_a:clean()

print("clean exit")

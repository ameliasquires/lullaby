require "llib"

llib.thread.lock(1)
llib.thread.lock(2)
llib.thread.unlock(2)

local thread_a = llib.thread.async(function (res)    
    --os.execute("sleep 1")
    --print((_G.ll + "hi"):final())
    print("waiting..")
    llib.thread.lock(1)
    print("signal!")
    res(_G.llib.crypto.md5())
    print("after")
end)

os.execute("sleep 1")
llib.thread.unlock(1)

awa = thread_a:await()

os.execute("sleep 1")
for i=1,999 do
print((awa + "hi"):final())
end

thread_a:clean()

print("clean exit")


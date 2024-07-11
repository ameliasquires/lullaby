llib = require "llib"

llib.config.set({print_meta=1,max_depth=22})
--llib.thread.lock(1)
--llib.thread.lock(2)
--llib.thread.unlock(2)
a = llib.thread.buffer(llib.crypto.md5())


print(a:get():final())
for i=1,2009 do
  llib.thread.async(function (res)    
    --llib.io.pprint(a);
    --l = a + a
    --a:mod(function(e) return e:update("meow") end)
  end)
end

--os.execute("sleep 1")
print(a:get():final())
--print("unlock")
--llib.thread.unlock(1)


--awa = thread_a:await()

--print(awa:await())
--print((awa + "hi"):final())
--thread_a:clean()

print("clean exit")

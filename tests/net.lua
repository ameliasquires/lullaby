require "llib"
llib.config.set({max_depth=5})
--local print = llib.io.pprint
sleep = function(a) 
    local sec = tonumber(os.clock() + a); 
    while (os.clock() < sec) do 
    end 
end
aea = 5
local wowa = 5
--_G.wo = llib
_G._llib = _G.llib
--_G.ww = llib
--llib.io.pprint(_G)
llib.net.listen(
    function(server)
        --llib = nil
        --llib.io.pprint(_G)
        llib.io.pprint("online")
        _G.server = server
        server:all("/", function(res, req)
            b = llib.crypto.md5("hello")

            --llib.io.pprint(a + '5')
            res:send(b)
            --llib.io.pprint(res)
            --llib.io.pprint(res)
            --print(res.send)
            --res:send("hi");
            --res.Code = 201
            --wwo.sleep(1)
            --wwo.llib.io.pprint(wwo.sleep)
            --require "llib"
            --llib.io.pprint(_G)
            
            --_G.llib.io.pprint(_G.ww)
            --llib.io.pprint(_G.wo)
            --print("hi from first")
            --llib.io.pprint(llib.crypto.md5("hewwo"))
            --_G.sleep(1)
            --_G.llib.io.pprint(_G._G._G._llib.crypto.md5("hi"))
            --_G.llib.io.pprint(_G._Go)
            --_G.llib.io.pprint(_G.wo.crypto.md5("55"))
            --_G.llib.io.pprint(req)
            --print(req.partial)
            --_G.llib.io.pprint(_G.llib.io.readfile(".gitignore"))
            --print(req._bytes)
            --res:send(_G.llib.io.readfile("llib.dll"))
            --_G.llib.io.pprint(_G.llib.crypto.md5(_G.llib.io.readfile(".gitignore")))
            --_G.llib.io.pprint(req)
            --_G.llib.io.pprint(req)
            --print("start")
            a = req:roll()
            --print(a)
            while a > -1 do 
                a = req:roll()
                print(req._bytes .. "/" .. req["Content-Length"])
                --print(a)
            end
            llib.io.pprint(req)
            --_G.llib.io.pprint(req)
            --_G.llib.io.pprint("hi")
            --res:send("done")
        end)

        server:GET("/aa", function(res, req)
            res.header["Content-Type"] = "text/plain"
            res:sendfile("readme.md")
        end)

        server:GET("/test55", function(res, req)
            res.Code = 403
            res:send("<h2>you would never</h2>")
        end)


        
    end,
    arg[1]
)

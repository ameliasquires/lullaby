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
        server:all("/*", function(res, req)
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
            _G.llib.io.pprint(req)
            print(req:roll(2000))
            _G.llib.io.pprint(req)
            res:send("owo")
        end)

        server:GET("/aa", function(res, req)
            --[[res.header["Content-Type"] = "text/plain"
            _G.server:lock()
            res:write("hi\n")
            res:write("next")
            _G.server:unlock()
            res:close()]]
            res.header["Content-Type"] = "text/plain"
            res:send(_G.llib.io.readfile("tests/net.lua"))
        end)

        server:GET("/test", function(res, req)
            res.Code = 403
            res:send("<h2>you would never</h2>")
        end)

        
    end,
    arg[1]
)

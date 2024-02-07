require "llib"
llib.config.set({max_depth=1})
--local print = llib.io.pprint
sleep = function(a) 
    local sec = tonumber(os.clock() + a); 
    while (os.clock() < sec) do 
    end 
end
aea = 5
_G.ww = llib
--llib.io.pprint(_G)
llib.io.pprint(llib.net.listen(
    function(server)
        print("wowa")
        server:all("/*", function(res, req)
            --llib.io.pprint(res)
            --llib.io.pprint(res)
            --print(res.send)
            --res:send("hi");
            --res.Code = 201
            --wwo.sleep(1)
            --wwo.llib.io.pprint(wwo.sleep)
            require "llib"
            llib.io.pprint(_G)
            llib.config.set({max_depth=4})
            llib.io.pprint(_G.ww)
            --print("hi from first")
            --llib.io.pprint(llib.crypto.md5("hewwo"))
        end)

        server:GET("/aa", function(res, req)
            res.header["Content-Type"] = "text/plain"
            server:lock()
            res:write("hi\n")
            res:write("next")
            res:close()
        end)

        server:GET("/test", function(res, req)
            res.Code = 403
            res:send("<h2>you would never</h2>")
        end)

        
    end,
    8080  
))

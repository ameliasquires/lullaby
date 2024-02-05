require "llib"
llib.config.set({max_depth=5})
--local print = llib.io.pprint
sleep = function(a) 
    local sec = tonumber(os.clock() + a); 
    while (os.clock() < sec) do 
    end 
end
aea = 5

--llib.io.pprint(_G.sleep)
llib.io.pprint(llib.net.listen(
    function(server)
        print("wowa")
        server:GET("/", function(res, req)
            --llib.io.pprint(res)
            --llib.io.pprint(res)
            --print(res.send)
            --res:send("hi");
            --res.Code = 201
            --wwo.sleep(1)
            --wwo.llib.io.pprint(wwo.sleep)
            require "llib"
            llib.io.pprint(req)
            print("hi from first")
        end)

        server:GET("/", function(res, req)
            require "llib"
            res.header["Content-Type"] = "text/plain"

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

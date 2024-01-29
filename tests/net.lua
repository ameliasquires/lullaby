require "llib"
llib.config.set({max_depth=5})
--local print = llib.io.pprint
function sleep (a) 
    local sec = tonumber(os.clock() + a); 
    while (os.clock() < sec) do 
    end 
end
llib.io.pprint(llib.net.listen(
    function(server)
        print("wowa")

        llib.io.pprint(server:GET("/", function(res, req)
            --llib.io.pprint(res)
            --llib.io.pprint(res)
            --print(res.send)
            --res:send("hi");
            --res.Code = 201
            sleep(1)
            res:send("<h2>hello world</h2>")
        end))

        llib.io.pprint(server:GET("/test", function(res, req)
            res.Code = 403
            res:send("<h2>you would never</h2>")
        end))

        
    end,
    8080  
))
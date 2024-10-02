--yes, this is kinda a mess of requires, just proof it works:3
net = require "lullaby.net"
io = require "lullaby.io"
crypto = require "lullaby.crypto"
config = require "lullaby.config"

config.set({max_depth=5})

sleep = function(a) 
    local sec = tonumber(os.clock() + a); 
    while (os.clock() < sec) do 
    end 
end

net.listen(
    function(server)

        io.pprint("online")
        _G.server = server

        server:POST("/{name}", function(res, req)
          --print("name is "..req.name)
          print("name")
          io.pprint(req.paramaters)
          res:stop()
        end)

        server:all("/{name}/nya/{user}", function(res, req)
          --print("name is "..req.name)
          print("name user")
          io.pprint(req.paramaters)
        end)

        server:all("*", function(res, req)
          print("all")
          io.pprint(req.paramaters)
        end)

        server:all("/{name}/user/*/{id}", function(res, req)
          print("owo")
          io.pprint(req.paramaters)

        end)

        server:all("/", function(res, req)
            b = crypto.md5("hello")

            res:send(b)
            
            a = req:roll()

            while a > -1 do 
                a = req:roll()
                print(req._bytes .. "/" .. req["Content-Length"])
            end
            io.pprint(req)

        end)

        server:GET("/aa", function(res, req)
            --res.header["Content-Type"] = "text/plain"
            res:sendfile("readme.md", {attachment=true})
        end)

        server:GET("/test55", function(res, req)
            res.Code = 403
            res:send("<h2>you would never</h2>")
        end)

        server:GET("/error", function(res, req)
            res.a.a.a.a()
        end)


        
    end,
    arg[1]
)

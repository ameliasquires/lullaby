local bserver = llby.thread.buffer(nil)

local net = llby.thread.async(function(tres)
  llby.net.listen(function(server)
    _G.server = server
    bserver:set(server)

    server:GET("/c", function(res, req)
      _G.server:GET("/test", function(res, req)
        res:send("555221")
      end)
    end)
  end, PORT)
end)

--should wait for the server to start
os.execute("sleep 0.1")

llby.net.request(string.format("localhost:%i/c", PORT))
local s = llby.net.request(string.format("localhost:%i/test", PORT))
local num = 2--s.content:read()

bserver:get():close()

net:await()
return tonumber(num) == 555221

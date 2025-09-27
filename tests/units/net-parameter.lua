local bserver = llby.thread.buffer(nil)

local a = 255
local b = 992

local net = llby.thread.async(function(tres)
  local c = 943
  llby.net.listen(function(server)
    bserver:set(server)
    server:GET("/{a}/{b}/{c}", function(res, req)
      res:send(tostring(tonumber(req.parameters.a) * a + tonumber(req.parameters.b) * b + tonumber(req.parameters.c) * c))
    end)
  end, PORT)
end)

local t1 = 293
local t2 = 928
local t3 = 777

--should wait for the server to start
os.execute("sleep 0.1")

local s = llby.net.request(string.format("localhost:%i/%i/%i/%i", PORT, t1, t2, t3))
local num = s.content:read()

bserver:get():close()

net:await()
return tonumber(num) == (t1 * a + t2 * b + t3 * 943)

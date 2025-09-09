PORT = 5552
local a = 255
local b = 992

local net = llby.thread.async(function(tres)
  local c = 943
  llby.net.listen(function(server)
    server:GET("/", function(res, req)
      res:send(tostring(tonumber(req.query.t1) * a + tonumber(req.query.t2) * b + tonumber(req.query.t3) * c))
    end)
  end, PORT)
end)

local t1 = 293
local t2 = 928
local t3 = 777

--should wait for the server to start
os.execute("sleep 0.1")

local s = llby.net.request(string.format("localhost:%i/?t1=%i&t2=%i&t3=%i", PORT, t1, t2, t3))
local num = s.content:read()

net:close()
return tonumber(num) == (t1 * a + t2 * b + t3 * 943)

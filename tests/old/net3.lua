--(this is in tests/old/net3.lua)
net = require "lullaby.net"
crypto = require "lullaby.crypto"

local server = net.server()

server:POST("/", function(res, req)
  --creates a sha0 hash object
  local hash = crypto.sha0()

  req:load()

  --incremental hashes allow updating via addition, in this case adding the body and getting a string from it
  hash = (hash + req.body):digest()
  --send the hash to the client, closes connection, but thread is live until it ends
  res:send(hash:hex())
end)

server:listen(8080)

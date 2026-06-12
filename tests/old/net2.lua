--(this is in tests/old/net2.lua)
net = require "lullaby.net"
local crypto = require "lullaby.crypto"
local port = 8080

net.listen(function(server)

  --listen to post requests at localhost:8080 (root directory)
  server:POST("/", function(res, req)
    --creates a sha0 hash object
    local hash = crypto.sha0()

    req:load()

    --incremental hashes allow updating via addition, in this case adding the body and getting a string from it
    hash = (hash + req.Body):final()
    print(hash, crypto)
    --send the hash to the client, closes connection, but thread is live until it ends
    res:send(hash)
  end)

end, port)

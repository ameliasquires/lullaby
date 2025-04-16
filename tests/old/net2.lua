--(this is in tests/net2.lua)
net = require "lullaby.net"
local crypto = require "lullaby.crypto"
local port = 8080
MAX_LENGTH = 2048

net.listen(function(server)

  --listen to post requests at localhost:8080 (root directory)
  server:POST("/", function(res, req)
    --creates a sha0 hash object
    local hash = crypto.sha0()
    --loads an extra 2048 characters from the request body (the body is not guaranteed to be >= 2048 characters, reasoning in docs)
    req:roll(MAX_LENGTH)

    --incremental hashes allow updating via addition, in this case adding the body and getting a string from it
    hash = (hash + req.Body):final()
    print(hash, crypto)
    --send the hash to the client, closes connection, but thread is live until it ends
    res:send(hash)
  end)

end, port)


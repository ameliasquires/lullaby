# lullaby

> a large multi-purpose library for lua (primarily 5.4) revolving around threading and cryptography, and including network, math, io, and table functions

heres an example of a webserver to return a [sha0](https://en.wikipedia.org/wiki/SHA-0) hash of an input

<blockquote>

```lua
--(this is in tests/net2.lua)
net = require "lullaby.net"
crypto = require "lullaby.crypto"
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
    --send the hash to the client, closes connection, but thread is live until it ends
    res:send(hash)
  end)

end, port)
```

</blockquote>

---

## building

build with `make`, output is `./lullaby.so` or (win)`./lullaby.dll`

windows works through msys2, install `mingw-w64-x86_64-lua` then run `make CC=gcc`

you can install with `doas make install` which will install lullaby.so into /usr/lib64/lua/5.X/

lua version can be specified with `version=...`, similar to 5.1, 5.3, jit, the default it 5.4

for working on the code base, i recommend using bear to generate compile_commands.json [(as outlined here)](https://clangd.llvm.org/installation#compile_commandsjson) which should probably let your ide find the headers

---

[some docs](docs/)

## todo:

* better tests

* rewrite docs

    * net mostly complete

----

# credits

* [luaproc](https://github.com/askyrme/luaproc) helped wrap my head around multiple lua_State concepts

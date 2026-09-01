# lullaby

> yes, i know this branch has far outlived reason for it to exist, there are a lot of large changes here, it will be merged when im more certain about the saftey of these new changes

> version 5.5 does work but will not be the primary target until it gets better distro support (and/or i feel like it)

> a large multi-purpose library for lua (primarily 5.4) revolving around threading and cryptography, and including network, math, io, and table functions

heres an example of a webserver to return a [sha0](https://en.wikipedia.org/wiki/SHA-0) hash of an input

<blockquote>

```lua
--(this is in tests/old/net3.lua)
net = require "lullaby.net"
crypto = require "lullaby.crypto"

local server = net.server()

server:POST("/", function(res, req)
  --creates a sha0 hash object
  local hash = crypto.sha0()

  req:load()

  --incremental hashes allow updating via addition, in this case adding the body and getting a string from it
  hash = (hash + req.body):final()
  --send the hash to the client, closes connection, but thread is live until it ends
  res:send(hash)
end)

server:listen(8080)
```

</blockquote>

---

## building

> im planning on making this better at somepoint!! along with better documention

build with `make`, output is `./lullaby.so` or (win)`./lullaby.dll`

windows may work through msys2, install `mingw-w64-x86_64-lua` then run `make CC=gcc` it is not tested enough currently

you can install with `doas make install` which will install lullaby.so into /usr/local/lib/lua/5.X

install directory can be configured with `INSTALL=...` which defaults to /usr/local/lib/lua/, but may be wanted in /usr/lib64/lua/

lua version can be specified with `version=...`, similar to 5.1, 5.2, 5.3, jit, or 5.5, the default it 5.4

for working on the code base, i recommend using bear to generate compile_commands.json [(as outlined here)](https://clangd.llvm.org/installation#compile_commandsjson) which should probably let your ide find the headers

---

[some docs](docs/)

## todo:

* better tests

* rewrite docs
    
* portability (memmem)

* like a ton of other stuff i have planned, meow

----

# credits

* [luaproc](https://github.com/askyrme/luaproc) helped wrap my head around multiple lua_State concepts

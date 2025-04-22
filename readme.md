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

note: any net code is prone to memory leaks, this will be addressed and fixed soon, on the other hand it has no know memory issues

---

~~net code also has some recv issues, ill fix these when i get time:3 only issue so far is with file parsing but i plan to rewrite alot of stuff~~

update:

recv code has been rewritten and i have fixed file parsing (with some testing) file parsing may be rewritten but works as of now

## building

build with `make`, output is `./lullaby.so` or (win)`./lullaby.dll`

windows works through msys2, install `mingw-w64-x86_64-lua` then run `make CC=gcc`

lua version can be specified with `version=...`, similar to lua5.1, lua5.3, luajit, the default it lua5.4

for working on the code base, i recommend using bear to generate compile_commands.json [(as outlined here)](https://clangd.llvm.org/installation#compile_commandsjson) which should probably let your ide find the headers

---

[some docs](docs/)

## todo:

* (working on seperatley) gui for graphs

* finish up http server

    * https

    * ~~check memory saftey~~ (*should* be good) (now work on indirect & more lifetime stuff)

    * memory optimizations (its getting there)

    * settings (parse/read files, etc..)

    * define usage of new routes inside of routes, and allow route removal

    * connection limit

    * allow choosing what to copy over to the thread, or not to copy the global state at all

    * allow stopping the server

* more doxygen like docs, everywhere

* encode tests (and fix sprintf ub)

----

# credits

* [luaproc](https://github.com/askyrme/luaproc) helped with multithreading


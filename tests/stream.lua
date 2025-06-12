local llby = require"lullaby"

--local a = llby.test.stream()
--print(a:file("test.awa", 200))

local req = llby.net.srequest("https://amyy.cc")

req.content:file("response.txt")

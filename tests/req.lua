local awa = require "lullaby"

print(awa.net.srequest("amyy.cc", "/", "meow").content)

--awa.net.srequest("example.com", "/", "meow")

--[[

net.srequest("example.com", "/", {key="meow"}, {Header="value"})

]]--

llib = require "lullaby"
llib.io.print_meta = 1

local a = llib.crypto.sha1()

local b = llib.thread.buffer(a)

llib.io.pprint(a)
llib.io.pprint(b:get())
llib.io.pprint((b + "meow"):final())


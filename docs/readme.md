# lullaby (llib) 
(name subject to change)

with the library in the same directory [(or one of the other valid search locations)](https://www.lua.org/pil/8.1.html)

```lua
llib = require "lullaby"
```

returns a table has many subtables and functions, with related function in them, you can view them like so

```lua
llib.io.pprint(llib) --pprint is a part of the io module, pprint meaning pretty print
```

all subtables and functions have a corresponding file in this directory on usage

you can also select just a specific module

```lua
crypto = require "lullaby.crypto"
crypto.sha224()
```

---

## big changes

### __clone metamethod (todo)

takes a single argument (the object to be cloned) returns a 'copy' of the object

this is for cloning a object to be the same, but not share any internals 

created for luaI_deepcopy (see src/lua.c) too create a seperate object for the other state

# lullaby (llby) 
(name subject to change)

with the library in the same directory [(or one of the other valid search locations)](https://www.lua.org/pil/8.1.html)

```lua
llby = require "lullaby"
```

returns a table has many subtables and functions, with related function in them, you can view them like so

```lua
llby.io.pprint(llby) --pprint is a part of the io module, pprint meaning pretty print
```

all subtables and functions have a corresponding file in this directory on usage

you can also select just a specific module

```lua
crypto = require "lullaby.crypto"
crypto.sha224()
```

### configuration

modules will each have different values you can change

each individual function will list what values you can modify, but all of them will exist in the parent table

```lua
llby.net.mimetypes = "/etc/meow"
```

## common functions/utils

### stream

streams allow generating a string in smaller chunks to save on memory

currently can only be made in c using luaI_newstream

stream:file(filename, bytes?)
stream:read(bytes?)

both function identically, file sending the data to a file and read returning it

the number of bytes can be selected, the function will return an amount close to what was requested

some streams may not support the bytes param, and may just ignore it. if it is ignored or not given it will always read the entire stream

---

## big changes

### __clone metamethod (todo)

takes a single argument (the object to be cloned) returns a 'copy' of the object

this is for cloning a object to be the same, but not share any internals 

created for luaI_deepcopy (see src/lua.c) too create a seperate object for the other state

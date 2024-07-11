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


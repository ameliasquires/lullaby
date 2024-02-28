# lualib (llib) 
(name subject to change)

with the library in the same directory [(or one of the other valid search locations)](https://www.lua.org/pil/8.1.html)

```lua
require "llib"
```

which makes a global llib table

> ### future require versions will eventually return the table
> ```lua
> llib = require "llib"
> ```

the table has many subtables, with related function in them, you can view them like so

```lua
llib.io.pprint(llib) --pprint is a part of the io module, pprint meaning pretty print
```

all subtables have a corresponding file in this directory, with info on its functions

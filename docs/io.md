# io 

## common 

### pprint

'accepts (probably) anything

formats input as a readable string

```lua
llib.io.pprint({a = 5, b = {9, 9, 22}})
```

#### config options

- print_type (0 | true) - whether or not to print item type (0 or 1)
- max_depth (2) - maximum depth that will not be collapsed
- start_nl_at (3) - maximum depth that will be kept in-line
- collapse_all (0 | false) - skip all newlines

### error/warn/log/debug 

'accepts a string 

outputs a fancy string (color!!!!)

```lua
llib.io.log("meow")
```

### readfile 

'accepts a file path

returns the content of that file as a string 

```lua
llib.io.readfile("./docs/io.md") -- (this file)
```
#### config options 

- file_chunksize (512) - size of chunk to be allocated


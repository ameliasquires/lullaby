# net

## listen (PARTIALLY IMPLEMENTED)

'takes a function with 1 argument and a integer for a port

the function will be ran on initilization, the argument has info on the server and functions to set it up

```lua
llib.net.listen(function(server)
    ...
end, 80)
```

### server:GET

'takes a string (the path) and a function to be ran in the background on request

the function has 2 arguments, the first (res) contains functions and info about resolving the request,
the second (req) contains info on the request

```lua
...
server:GET("/", function(res, req) do
    ...
end)
...
```

#### res:send

'takes a string 

sends the string to the client

```lua
...
res:send("<h1>hello world</h1>")
...
```

#### res:set **

'takes 2 strings, key and value

set the key to value in the response header, certain keys will affect other values or have other side effects on res:send, listed below

|key|side effect|
|--|--|
|Code|Changes response note, ie: (200: OK)|


```lua
...
res:set("Content-Type", "text/html") -- Content-Type: text/html
...
```

#### res:close()

closes connection


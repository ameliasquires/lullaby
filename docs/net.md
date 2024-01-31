# net

## listen (PARTIALLY IMPLEMENTED)

'takes a function with 1 argument and a integer for a port

the function will be ran on initilization, the argument has info on the server and functions to set it up

**
right now everything within a server:GET function is completley local, cannot access the global context at all,
i am planning on copying the global state to each thread (optionally ofc) to make the global state read only, and maybe
and option to sync the global state (push and pull seperately)
**

```lua
llib.net.listen(function(server)
    ...
end, 80)
```

### server:lock server:unlock **

continues on the current thread, but pauses all other threads at that point

```lua
...
server:lock()
--do something with a global
server:unlock()
...
```

### server:close **

closes server

### server:use

'takes a function with 3 paramaters

first and second are res and req as described in server:GET, the third is a function to move to the next point, executes in the order given and can be chained

```lua
server:use(function(res, req, next) 
   if(req['Version'] == "HTTP/1.1") then 
    next()
   end
end)

server:GET("/", function(res, req)
    --version will always be 1.1, as per the middleware
end)
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

#### res:deny **

denies request as if there was no server 

```lua
...
res:deny() --make the client timeout, lol
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

#### res:close **

closes connection

#### res.header

table containing all head information, anything added to it will be used, certain keys will affect other values or have other side effects on res:send, listed below

|key|side effect|
|--|--|
|Code|Changes response note, ie: (200: OK)|

```lua
...
res.header["Code"] = 404
res.header["test"] = "wowa"
-- new header will have a code of 404 (at the top duh)
-- and a new field "test"
--
-- HTTP/1.1 404 Not Found
-- ...
-- test: wowa
...
```

### server:static_serve **

'takes two strings, first is server serve path, second is local path, being a file or directory

```lua
...
server:static_serve("/public", "./html/")
...
```


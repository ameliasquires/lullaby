# net

## listen (PARTIALLY IMPLEMENTED)

'takes a function with 1 argument and a integer for a port

(intentionally styled after expressjs:3)
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

### server:lock server:unlock

continues on the current thread, but pauses all other threads at that point

```lua
...
server:lock()
--do something with a global
server:unlock()
...
```

### server:close

closes server

### server:GET/POST/...

'takes a string (the path) and a function to be ran in the background on request

the function has 2 arguments, the first (res) contains functions and info about resolving the request,
the second (req) contains info on the request, the path allows for wildcards, multiple get requests per path is allowed

the actual name of the function will change based on what request method you want to accept, all requests are treated the exact same on the backend, besides HEAD requests which will also use all GET requets, and the 'all' variant will get everything

```lua
server:all("*", function(res, req, next) 
   if(req['Version'] ~= "HTTP/1.1") then 
      res:close()
   end
end)

...
server:GET("/", function(res, req) do
    --version will always be 1.1, as per the middleware
    ...
end)
...
```

#### res:write

'takes a string 

sends the string to the client, constructs a header on first call (whether or not res.header._sent is null)
(the constructed header can not be changed later on in the request), and sends the string without closing the client

```lua
...
res:write("<h1>hello world</h1>")
res:write("<h1>good bye world</h1>")
...
```

#### res:send

'takes a string 

sends the string to the client, constructs a header then closes client_fd

```lua
...
res:send("<h1>hello world</h1>")
...
```

#### res:close

closes connection, sets res.client_fd to -1, any calls that use this value will fail

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

### res:serve **

'takes one string, which is a path that will be served, file or dir

```lua
...
res:serve("./html/")
...
```

### req:roll **

'takes an integer of bytes to read & parse

will update req according to how the bytes needed to be parsed

return chart
|code|meaning|
|--|--|
|0|no more to read|
|1|finished file/chunk, may be more|

(may add more)

```lua
--when a request contains "hello world"
req.Body --"hello"
req:roll(30) --does not matter if you go over
req.Body --"hello world
```


# net

## listen (mostly IMPLEMENTED)

'takes a function with 1 argument and a integer for a port

(intentionally styled after expressjs:3)
the function will be ran on initilization, the argument has info on the server and functions to set it up

**
right now everything within a server:GET function is partially global, it can read global variables (by making a copy),
it can not read/copy local variables or modify globals
**

allows a third optional argument that offers some other options in a table format

|name|default value|extra info|
|--|--|--|
|mime.types.file**|/etc/mime.types|formated similarly to [this](https://wiki.debian.org/MIME/etc/mime.types)|
|max.connections**|64|maximum number of connections that can be opened at the same time, will respond with error(503)|
|max.header.size**|1<<20 (1048576)|max header size before refusing connection with error(431)|
|max.uri.size**|idk yet|maximum uri size before refusing request with error(414)|
|max.request.timeout**|idk yet|maximum time server will wait for request to be parsed|


the server will send these codes for these reasons
|code|cause|
|--|--|
|503**|too many current requests, more than max.connections|
|500**|anytime a server route crashes|
|431**|header is larger than max header size, more than max.header.size|
|414**|request uri is longer than max.uri.size|
|408**|request took too longer than max.request.timeout|
|404**|request has no defined route|

(more to come?**)

```lua
llib.net.listen(function(server)
    ...
end, 80)
```

```lua
llib.net.listen(function(server)
    ...
end, 80, {["mime.types.file"] = "/etc/mime.types"})
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

### server:close **

closes server

### server:GET/POST/...

'takes a string (the path) and a function to be ran in the background on request

the function has 2 arguments, the first (res) contains functions and info about resolving the request,
the second (req) contains info on the request, the path allows for wildcards, multiple get requests per path is allowed

it also allows for path paramaters which is a wildcard directory that pushes the results into req.paramaters (read below)

the actual name of the function will change based on what request method you want to accept, all requests are treated the exact same on the backend, besides HEAD requests which will also use all GET requets, and the 'all' variant will get everything

```lua
server:all("*", function(res, req) 
   if(req['Version'] ~= "HTTP/1.1") then 
      res:stop()
   end
end)

...
server:GET("/", function(res, req)
    --version will always be 1.1, because the request passes through the function sequentially
    ...
end)
...

server:GET("/home/{user}/id", function(res, req)
    --sets req.paramaters.user to whatever path was requested
end)
```

#### res:write

'takes a string 

sends the string to the client, constructs a header on first call (whether or not res.header._sent is null)
(the constructed header can not be changed later on in the request*), and sends the string without closing the client

```lua
...
res:write("<h1>hello world</h1>")
res:write("<h1>good bye world</h1>")
...
```

*well it can but it wont do anything 

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

this will still run any selected functions! 

#### res:stop 

prevents all further selected functions from running

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

### res:sendfile

'takes one string, which is a path that will be served, must be a file

res.header["Content-Type"] is set automatically depending on the file extention, using /etc/mime.types, or whatever option was supplied to listen (see listen options)

```lua
...
res:sendfile("./html/index.html")
...
```

### req.paramaters 

a list of paramaters for the current function 

a path of '/user/{name}/id'
and a request of '/user/amelia/id'
would set req.paramaters.name to amelia

currently you can not have multiple paramaters per directory

> this could be changed in later versions

/home/{name} is valid 

/flight/{id}-{airline} is not

these can, of course be used with wildcards however you want

/*/{user}/id would match /a/b/c/meow/id with req.paramaters.user being equal to meow

### req:roll

'takes an integer of bytes to read & parse (optional, otherwise the max amount of bytes will be read)

will update req according to how the bytes needed to be parsed, returns the number of bytes read (not necessarily parsed), 0 if there
is no more ready data, -1 if all data has been read, and any other values \< -1 is a recv error (add 1 to the error code)

```lua
--when a request contains "hello world"
req.Body --"hello"
req:roll(30) --does not matter if you go over, returns 7 (probably)
req.Body --"hello world"
req:roll(50) --returns -1, no more to read 
--req.Body has not been updated
```

can have unique results with files (this example is not perfect, roll could read less than 2 bytes, and this does not account for newlines)

```lua
--sent a file ina request to the server, where the boundary is 'abcd':
--  ---abcd
--  (header junk, file name and stuff)
--  
--  wowa
--  --
--  --ab
--  --abcd

--when the line 'wowa' has just been read, using roll for less than two will not update the file
req.Body -- (...)"wowa"
req:roll(2)
req.Body -- (...)"wowa" (unchanged)
--any lines that contain just the boundary and -'s will be put in a seperate buffer until it ends or breaks
--a previous condition, then it will be added
req:roll(2)
req.Body -- (...)"wowa\n--"
--and now "--" (from the next line) is in the possible boundary buffer, it ends in "ab" so it will be added to the main body
```


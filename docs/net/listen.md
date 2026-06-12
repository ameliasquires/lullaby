## listen (mostly IMPLEMENTED)

net.listen(function, port)

currently does not work with any transfer-encoding!!

(intentionally styled after expressjs:3)
the function will be ran on initilization, the argument has info on the server and functions to set it up

if the 'server' upvalue needs to be accessible inside of the network threads, you will need to make the value global or redefine it. the upvalue itself (as a argument to the listen function) will simply not exist when the route functions are called, despite it looking like they would be.
because the route functions (GET, POST, etc..) are just assigning functions to these paths, and not running them, they will just not see the server value.
this also explains another weird behaviour where routes can read locals before they've been defined. (the upvalues of the functions are defined when the listen function is complete)

```lua
llby.net.listen(function(server)
  _G.server = server

  server:GET("/", function(res, req)
    --first will always be null, second will be null without the second line
    print(server, _G.server)

    --will be valid despite being defined later
    print(awa)
  end)

  local awa = 22
end)
```

|name|default value|extra info|
|--|--|--|
|mimetypes|/etc/mime.types|file used to auto assign content-type when using res:sendfile, nil to skip|
|max_connections**|64|maximum number of connections that can be opened at the same time, will respond with error(503)|
|max_header**|1<<20 (1048576)|max header size before refusing connection with error(431)|
|max_uri**|idk yet|maximum uri size before refusing request with error(414)|


the server will send these codes for these reasons
|code|cause|
|--|--|
|503|too many current requests, more than max_connections|
|500|anytime a server route crashes|
|431|header is larger than max header size, more than header_size|
|414|request uri is longer than max_uri|

```lua
llby.net.listen(function(server)
    ...
end, 80)
```

### server.ssl

a table with two keys, expecting a crt and a key file path

```lua
server.ssl = {
    key = "server.key",
    crt = "server.crt"
}
```

### server:close

closes server, will not halt other already accepted requests

### server:GET/POST/...

server:GET(path, function)

the function has 2 arguments, the first (res) contains functions and info about resolving the request,
the second (req) contains info on the request, the path allows for wildcards, multiple get requests per path is allowed

it also allows for path parameters which is a wildcard directory that pushes the results into req.parameters (read below)

the actual name of the function will change based on what request method you want to accept, all requests are treated the exact same on the backend, besides HEAD requests which will also use all GET requets, and the 'all' variant will get everything

```lua
server:all("*", function(res, req) 
   if(req['version'] ~= "HTTP/1.1") then 
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
    --sets req.parameters.user to whatever path was requested
end)
```

#### res:write

res:write(content)

sends the string to the client, constructs a header on first call (whether or not res.header._sent is null)
(the constructed header can not be changed later on in the request*), and sends the string without closing the client

```lua
res:write("<h1>hello world</h1>")
res:write("<h1>good bye world</h1>")
```

*well it can but it wont do anything 

#### res:send

res:send(content)

sends the string to the client, constructs a header then closes client_fd

```lua
res:send("<h1>hello world</h1>")
```

functionaly identical to res:write and res:close

sets res.open to false

#### res:close

res:close()

closes connection, sets res.client_fd to -1, any calls that use this value will fail

this will still run any selected functions! 

this is called automatically when there are no more function

sets res.open to false

#### res:stop 

res:stop()

prevents all further selected functions from running

#### res.header

table containing all head information, anything added to it will be used, certain keys will affect other values or have other side effects on res:send, listed below
values should be in all lowercase

|key|side effect|
|--|--|
|code|Changes response note, ie: (200: OK)|
|content-type|this is changed automatically with res:sendfile|
```lua
res.header["code"] = 404
res.header["test"] = "wowa"
-- new header will have a code of 404 (at the top duh)
-- and a new field "test"
--
-- HTTP/1.1 404 Not Found
-- ...
-- test: wowa
```

### res:sendfile

res:sendfile(filepath, options?)

this can return an error if the file is not found or if the user does not have read permissions

res.header["content-type"] is set automatically (unless already set) depending on the file extention, using /etc/mime.types, or whatever option was supplied to listen (see listen options)

options table

|key|value|effect|
|--|--|--|
|attatchment|boolean, default false|whether or not to add content-disposition (if file will be downloaded)|
|filename|string, defualts as the first argument|what the client should see as a filename (only when attatchment is true)|


```lua
res:sendfile("./html/index.html")
```

sets res.open to false

### res:error

res:error(code)

sends an error message to the client

```lua
res:error(404)

-- HTTP/1.1 404 Not Found
-- ...
-- 
-- not found
```

sets res.open to false

### res:upgrade

res:upgrade()

returns an error if the upgrade is not available

reads the res.header["upgrade"] and updates the connection (for the rest of the routes too!!)
calling this will update, add or delete some res or req values/functions

#### res:upgrade (websocket)

removes:

req:load

adds:

res._ws - (data for the websocket)
res:send - sends data as a full frame
res:write** - sends data as an incomplete frame
res:sendfile** - sends file to the client
res:close** - same function

### res.open

boolean determining if the connection has been closed via res:send or res:close

### req.parameters 

a list of parematers for the current function 

a path of '/user/{name}/id'
and a request of '/user/amelia/id'
would set req.parameters.name to amelia

currently you can not have multiple parameters per directory

> this could be changed in later versions

/home/{name} is valid 

/flight/{id}-{airline} is not

these can, of course be used with wildcards however you want

/*/{user}/id would match /a/b/c/meow/id with req.parameters.user being equal to meow

### req:load

req:load(bytes?)

when no parameters are passed (or bytes is -1) the whole request will be read

otherwise, that many bytes will be read from the client and parsed

when the content has just a body and no files, the bytes will be loaded as they are parsed into req.body

when the content contains files, the files will be added to req.files when the full file is parsed (they will not be partial)

will update req according to how the bytes needed to be parsed, returns the number of bytes read (not necessarily parsed), 0 if there
is no more ready data, -1 if all data has been read, and any other values \< -1 is a recv error (add 1 to the error code)

returns 1 when there is still more data to parse
returns 0 when there is not more data to parse
may return an error

> waiting on a rewrite for this, all that will be changed will be how errors are returned

```lua
--when a request contains "hello world"
req.body --"hello"
req:load(30) --does not matter if you go over, returns 7 (probably)
req.body --"hello world"
req:load(50) --returns -1, no more to read 
--req.body has not been updated
```

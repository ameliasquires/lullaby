## ws

net.wss(url)
net.ws(url)**

both function identically, wss just uses openssl over socket

will call each other when the url protocol mismatches the function

can return an error

```lua
net.wss("amyy.cc") -- connects to wss://amyy.cc
net.wss("ws://amyy.cc") -- identical to net.wss("amyy.cc")
```

```lua
local con = net.ws("amyy.cc")

while true do
    local frame = con:read()
    print(frame.content)
end
```

### con:read

con:read()

reads the oldest unread frame from the server or wait for the next frame

can return an error

return table has the frame content (.content) and opcode (.opcode)

### con:write

con:write(content)

sends a frame, returns nil or an error

### con:close

con:close()

calls __gc early

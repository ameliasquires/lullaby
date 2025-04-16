llby = require("lullaby")

local ws, error = llby.net.wss("echo.websocket.org:4432")

if ws == nil then
  print(error)
  os.exit(12)
end

print(ws)

for i=1,10 do
  local c = ws:read().content
  print(c)
  ws:write(c.."a")
end

ws:close()
ws:close()

--ws:ping()
--add onclose?

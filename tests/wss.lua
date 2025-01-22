llby = require("lullaby")

local ws = llby.net.wss("echo.websocket.org", 443)

for i=1,500 do
  local c = ws:read()
  print(c)
  ws:write(c.."a")
end
ws:close()

--ws:ping()
--add onclose?

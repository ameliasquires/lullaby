llby = require"lullaby"

llby.net.listen(function(server)
  server:GET("/", function(res, req)
    res:sendfile("license.md")
    --res:sendfile("../awa/static/volcarona.gif")
  end)
end, 8888)

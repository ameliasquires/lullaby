llby = require "lullaby"

llby.net.listen(function(server)
  server:GET("/", function(res, req)
    print("hi")
    llby.io.pprint(req)
  end)
end, 8887)

local llby = require"lullaby"

local r
function c()
  r = llby.net.srequest("https://evil")
llby.error.is(r):raise()

end

function b()
  c()
end

function a()
  b()
end

a()


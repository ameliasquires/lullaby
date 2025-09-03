local lvar = 224400
_G.gvar = 33220

function lfun(a, b)
  return (a + lvar) * (b + gvar)
end

local thread = llby.thread.async(function(res)
  res(lfun(lvar, gvar))
end)

return thread:await() == lfun(lvar, gvar)

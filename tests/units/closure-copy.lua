local A = 298
local B = 3428
local C = 438
local D = 4444
local function outer(a, b)
  local c = C
  return function(d)
    return a + b * c + d
  end
end

local val = llby.thread.async(function(res)
  res(outer(A, B)(D))
end):await()

return val == A + B * C + D

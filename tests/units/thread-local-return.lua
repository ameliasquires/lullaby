local a = 224

local thread = llby.thread.async(function(res)
  local b = 948

  res(function(c)
    return a * b * c
  end)
end)

local fun = thread:await()

return fun(448) == 224 * 948 * 448

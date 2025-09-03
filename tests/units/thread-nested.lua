local mutex = llby.thread.mutex()
mutex:lock()

local t1 = llby.thread.async(function(res)

  local t2 = llby.thread.async(function(res)

    local t3 = llby.thread.async(function(res)
      mutex:lock()

      res(254)
    end)


    res(t3)
  end)

  res(t2)
end)

mutex:unlock()
return t1:await():await():await() == 254

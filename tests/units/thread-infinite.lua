local pass = llby.thread.buffer(true)

local th = llby.thread.async(function(res)
  while true do
    res:testclose()
  end
end)

local readyb = llby.thread.buffer(false)

local th2 = llby.thread.async(function(res)
  llby.thread.usleep(1000 * 1000)
  if not readyb:get() then
    pass:set(false)
    th:kill()
  end
end)

th:close()
readyb:set(true)
th2:await()

return pass:get()

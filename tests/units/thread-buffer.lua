local h1 = llby.thread.buffer(llby.crypto.md5())
local h2 = llby.thread.buffer(llby.crypto.sha256())
local h3 = llby.thread.buffer(llby.crypto.sha1())

local tthread = llby.thread.async(function(res)
  h1:set(h1:get():update("mrrp"))
  h2:set(h2:get():update("mrrp"))

  h3:mod(function(M)
    return M:update("mrrp")
  end)

  res(h1:get(), h2:get(), h3:get())
end)

local h4, h5, h6 = tthread:await()

return h1:get():final() == h4:final() and h2:get():final() == h5:final() and h3:get():final() == h6:final()
    and h4:final() == "be40416e1491ae73fee43a0cf01132fa"
    and h5:final() == "a4ba2864e6dcc988c6df73cdfbee6d308e39174dd86dddc4e328c4f2df1c48e9"
    and h6:final() == "7bf0ffbd68005c35faa12f5ba6df54288969220c"

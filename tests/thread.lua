local llby = require("lullaby")

local fail = 0

local b1 = llby.thread.buffer(512)
b2 = llby.thread.buffer({1, 2, 3, 4, 5, 6, 7})
b3 = llby.thread.buffer(llby.crypto.md5())

llby.thread.async(function(res)
  b1:set(999)

  b2:mod(function(M)
    table.insert(M, 99)
    M.awa = 290
    return M
  end)

  b3:set(b3 + "uwu")
end):await()

if (b1:get() == 999) then llby.io.log(b1:get() .. " == 999")
else llby.io.error(b1:get() .. " != 999"); fail = fail + 1; end

if (b2:get()[8] == 99) then llby.io.log(b2:get()[8] .. " == 99")
else llby.io.error(b2:get()[8] .. " != 99"); fail = fail + 1; end

if (b2.awa == 290) then llby.io.log(b2.awa .. " == 290")
else llby.io.log(b2.awa .. " == 290"); fail = fail + 1; end

if (b3:get():final() == "174a3f4fa44c7bb22b3b6429cb4ea44c") then llby.io.log(b3:get():final() .. " == 174a3f4fa44c7bb22b3b6429cb4ea44c")
else llby.io.error(b3:get():final() .. " != 174a3f4fa44c7bb22b3b6429cb4ea44c"); fail = fail + 1; end

if fail == 0 then llby.io.log("no errors")
else llby.io.error(fail .. " errors") end

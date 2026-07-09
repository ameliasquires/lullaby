---@meta 

---@alias error nil, any

---used for handling lullaby errors
---@class err
local err = {}

---@class err-table
local err_table = {}

---@type boolean
err_table.error = true

---prints the error
---@return err-table
function err_table:print() end

---raises the lullaby error to a lua error
---@return err-table
function err_table:raise() end

---determines whether a return value is an error
---the return table functions will return immediately if there was no error
---@param type any
---@return err-table
function err.is(type) end

return err

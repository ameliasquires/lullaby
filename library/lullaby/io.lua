---@meta

---@class io
local io = {}

---print a string with a "pretty" log header
---@param value string value to print
---@return nil
function io.log(value) end

---print a string with a "pretty" warning header
---@param value string value to print
---@return nil
function io.warn(value) end

---print a string with a "pretty" error header
---@param value string value to print
---@return nil
function io.error(value) end

---print a string with a "pretty" debug header
---@param value string value to print
---@return nil
function io.debug(value) end

---prints any value, expanding tables
---@param value any value to print
---@return nil
function io.pprint(value) end

---@deprecated
function io.readfile() end

---@deprecated
function io.json_parse() end

---@deprecated
function io.arg_handle() end

return io

---@meta

---@class io
---@field filemallocchunk integer default chunk size for readfile
---@field print_type integer print the type next to the value
---@field max_depth integer max print depth before collapsing
---@field start_nl_at integer when to start new line while printing
---@field collapse_all integer whether or not to collapse everything
---@field collapse_to_memory integer when collapsing, print memory address
---@field print_meta integer print metatable
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

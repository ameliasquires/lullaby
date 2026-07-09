---@meta

---@class meta
local meta = {}

---@class stream
meta.stream = {}

---sends the rest of a streams contents to a file
---@param filename string
---@param bytes integer? max amount to read before stopping, 0 if nil
---@param mode string? what mode to open the file, w if nil
function meta.stream:file(filename, bytes, mode) end

---reads bytes from a stream
---@param bytes integer? max amount to read before stopping
---@return string
function meta.stream:read(bytes) end

return meta

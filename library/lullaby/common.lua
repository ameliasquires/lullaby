---@meta

---@class meta
local meta = {}

---@class stream
meta.stream = {}

---sends the rest of a streams contents to a file
---@param T stream
---@param filename string
---@param bytes integer? max amount to read before stopping
function meta.stream.file(T, filename, bytes) end

---reads bytes from a stream
---@param T stream
---@param bytes integer? max amount to read before stopping
---@return string
function meta.stream.read(T, bytes) end

return meta

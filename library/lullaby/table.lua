---@meta

---to be mostly rewritten
---@class lullaby.table
local table = {}

---splits string into smaller strings, removing the delimiter
---```lua
---llby.table.split("/hello/world//test///", "/") -- {"hello", "world", "test"}
---llby.table.split("/hello/world//test///", "/", false) -- {"hello", "world", "", "test", "", "", ""}
---```
---@param haystack string
---@param search string
---@param skip boolean? whether or not to skip empty chunks
---@return string[]
function table.split(haystack, search, skip) end

---compares 2 tables recursivley
---@param A any[]
---@param B any[]
---@param depth integer? max depth to search, defaults to -1
---@return boolean
function table.equal(A, B, depth) end

---clones table recursivley
---@param table any[]
---@return value[]
function table.dup(table) end

---gets table len, useful for key,value tables
---@param table any[]
---@return integer
function table.len(table) end

---greatest, least
---@deprecated
---@param array number[]
function table.quicksort(array) end

---greatest, least
---@deprecated
---@param array number[]
function table.mergesort(array) end

---greatest, least
---@deprecated
---@param array number[]
function table.shellsort(array) end

---greatest, least
---@deprecated
---@param array number[]
function table.bubblesort(array) end

---greatest, least
---@deprecated
---@param array number[]
function table.heapsort(array) end

return table

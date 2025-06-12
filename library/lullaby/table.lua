---@meta

---to be rewritten
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

---least, greatest
---@deprecated
---@param array integer[]
function table.countintsort(array) end

---dont use this lol
---@deprecated
---greatest, least
---@param array number[]
function table.miraclesort(array) end

---dont use this lol
---@deprecated
---greatest, least
---@param array number[]
function table.stalinsort(array) end

---dont use this lol
---@deprecated
---greatest, least
---@param array number[]
function table.slowsort(array) end

---dont use this lol
---@deprecated
---greatest, least
---@param array number[]
function table.bogosort(array) end

return table

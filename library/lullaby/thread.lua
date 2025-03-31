---@meta

---@class thread
local thread = {}

---@class async-table
local async = {}

---waits for thread to exit, returns the value of res
---@param T async-table
---@return ...
function async.await(T) end

---calls thread __gc early
---@param T async-table
---@return nil
function async.clean(T) end

---contains data for the thread
---@deprecated
---@type lightuserdata
async._ = nil

---@async
---@param fun fun(res: fun(...)): nil function to call, parameter will set a return value for the thread
---@return async-table
function thread.async(fun) end

---@class buffer-table
local buffer = {}

---gets the value of the buffer
---@param T buffer-table
---@return any
function buffer.get(T) end

---sets the value of the buffer
---@param T buffer-table
---@param value any
---@return nil
function buffer.set(T, value) end

---calls a function with a parameter that is the value of the buffer, return the new value of the buffer
---@param T buffer-table
---@param fun fun(any): nil
---@return nil
function buffer.mod(T, fun) end

---puts a value into a atomic thread-safe buffer
---@param value any
---@return buffer-table
function thread.buffer(value) end

---locks any thread reaching this lock id until a corresponding unlock is met
---@param tid integer
---@return nil
function thread.lock(tid) end

---unlocks a lock id
---@param tid integer
---@return nil
function thread.unlock(tid) end

---@deprecated
function thread.testcopy() end

return thread

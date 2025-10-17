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

---stops the thread, may be unavaliable on some systems (android) and will call async:kill instead
---@param T async-table
---@return nil
function async.close(T) end

---stops the thread forcefully, may cause problems, likely not thread-safe
---@param T async-table
---@return nil
function async.kill(T) end

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

---@class mutex-table
local mutex = {}

---locks the mutex
---@param T mutex-table
---@return nil
function mutex.lock(T) end

---unlocks the mutex
---@param T mutex-table
---@return nil
function mutex.unlock(T) end

---frees the mutex, automatically called by __gc
---@param T mutex-table
---@return nil
function mutex.free(T) end

---returns a mutex object, useful for solving race conditions in multi-threaded environments
---@return mutex-table
function thread.mutex() end

return thread

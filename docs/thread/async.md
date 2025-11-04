## async

thread.async(function(res))

despite the name, this function provides a new state running under a new thread. coroutines would be more what you are looking for for async

the argument function is executed by the thread until it has been completed. the res paramater is a function that takes any number of arguments and sets them as the value for the thread:await() call, and kills the thread (safely)

the reason for the res function it because lua_call (in the c api) requires a number or return values before you run the function

the res function also provides some child methods for thread managment

### res:testclose

res:testclose()

closes the thread if it is being requested to close from async:close

### res:autoclose

res:autoclose()

calls res:testclose() every lua line, using a debug hook

---

### async:await

async:await()

pauses the current thread until the selected thread exits, or calls res(), the return value is whatever the thread passed to res

### async:kill

async:kill()

kills the thread, this may close it in an unsafe way, and should be avoided

### async:close

async:close()

waits for the thread to internally call res:testclose or exit

### async:clean

async:clean()

calls the __gc metamethod, will call async:kill() if it is still running

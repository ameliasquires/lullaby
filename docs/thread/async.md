## async

thread.async(function(res))

despite the name, this function provides a new state running under a new thread. coroutines would be more what you are looking for for async

the argument function is executed by the thread until it has been completed. the res paramater is a function that takes any number of arguments and sets them as the value for the thread:await() call, and kills the thread (safely)

the reason for the res function it because lua_call (in the c api) requires a number or return values before you run the function


### async:await

async:await()

pauses the current thread until the selected thread exits, or calls res(), the return value is whatever the thread passed to res

### async:kill

async:kill()

kills the thread, this may close it in an unsafe way, and should be avoided

### async:close

async:close()

kills the thread in a more safe manor, should be preferred in most cases. however it is best to let the thread exit itself

some systems may not support this (pthread_cancel) and so this function will call async:kill() in cases where it cant. android is one of the main ones

### async:clean

async:clean()

calls the __gc metamethod, will call async:close() if it is still running

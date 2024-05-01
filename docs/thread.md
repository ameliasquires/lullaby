# threads **

## llib.thread **

'takes a function which will be ran in a separate thread with a single parameter with thread info

these have the same backend (and limitations) of network threads

```lua
local thread = llib.thread(function(info)
    local N = 0
    ...
    return N;
end)
```

### thread function parameters

as used with "info" above

#### info:send()

'takes "any" value

send a value which can be retrieved from outside the thread with thread:next()

```lua
info:send(5)
info:send("hello")
```

#### info:pause()

pauses the thread (must be resumed from outside)

### thread return object

#### thread:await()

'optional timeout

waits for the thread to return, and returns whatever it returned, if timeout is exceeded nil

```lua
thread:await() -- value of N (above)
```

#### thread:next()

gets the most oldest value sent using info:send() and pops it

```lua
--(continued from above)
thread:next() -- 5
thread:next() -- "hello"
```

#### thread:kill()

kills the thread

#### thread:pause() thread:resume()

stops or continues the thread

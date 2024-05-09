# threads **

## lock, unlock

'takes an integer

locks any other thread reaching this lock id until a corresponding unlock is met

```lua
llib.thread.lock(5)
...
llib.thread.unlock(5)
```

more indepth

```lua
llib.thread.lock(5)
local t = llib.thread.async(function(info)
    ...
    llib.thread.lock(5)
    ...
    res(N)
end)

...
llib.thread.unlock(5)
t:await()
```

## aync

'takes a function which will be ran in a separate thread with a single parameter with thread info

these have the same backend (and limitations) of network threads

the rej (2nd param) is currently unused

```lua
local thread = llib.thread.async(function(res, rej)
    local N = 0
    ...
    res(N)
end)
```

### thread function parameters

as used with "res" above

#### res()

'takes any amount of "any" values

send a value(s) to thread:await() call then stalls the thread until cleaned

### thread return object **

#### thread:await() **

'optional timeout in ms and boolean whether to keep or not

waits for the thread to return, and returns whatever it returned then closes it, or nil if timeout was exceeded

```lua
thread:await() -- value of N (above)
```

```lua
thread:await(20) -- value of N (above) or nil and preserves the thread
```

#### thread:clean() **

frees everything related to the thread (including userdata allocated in it!), thread:await() can not be called again, all lua values will still be usable

not required to be called, lua gc should call it on its own via __gc

#### thread:kill() **

kills the thread

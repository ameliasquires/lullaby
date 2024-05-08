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

## aync **

'takes a function which will be ran in a separate thread with a single parameter with thread info

these have the same backend (and limitations) of network threads

```lua
local thread = llib.thread.async(function(res, rej)
    local N = 0
    ...
    res(N)
end)
```

### thread function parameters **

as used with "res" above

#### res()

'takes any amount of "any" values

send a value(s) to thread:await() call then stalls the thread until cleaned

#### res:send() **

'takes "any" value

send a value which can be retrieved from outside the thread with thread:next()

```lua
res:send(5)
res:send("hello")
```

### thread return object **

#### thread:await() **

'optional timeout in ms and boolean whether to keep or not

waits for the thread to return, and returns whatever it returned then closes it, or nil if timeout was exceeded
if the input is the boolean value true, it will keep the thread alive (otherwise await() can not be called again)

```lua
thread:await() -- value of N (above)
```

```lua
thread:await(20) -- value of N (above) or nil
```

```lua
thread:await(true) -- value of N (above)
thread:await() -- same
thread:await() -- error, function above performed cleanup
```

#### thread:next() **

gets the most oldest value sent using info:send() and pops it

```lua
--(continued from above)
thread:next() -- 5
thread:next() -- "hello"
```

#### thread:kill() **

kills the thread

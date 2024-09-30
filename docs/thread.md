# threads **

## buffer

'takes 'anything'

a thread-safe object buffer to easily transfer things between threads 

you *can* index the buffer normally, but all data will be read only, and it does NOT work with the colon syntax

i might be able to fix this in the future with a proxy function solution, the difficulty arrises when i have no way to tell if it is being sent as a dot function 

```lua
buffer = llib.thread.buffer(llib.crypto.md5())
buffer.final(buffer:get()) --works fine, same as buffer:get().final(buffer:get()) 
buffer:final() --does not work, this *would* expands to buffer.final(buffer), but unless the functions expects my specific buffer object (which is dumb), it will break
```

also, be careful sending userdata, lightuser data cant really be copied, user data will be memcpy'd (or will use __clone)

full example:

```lua
buffer = llib.thread.buffer({2, 3, 4})
buffer:get() --{2, 3, 4}
...
buffer:set({3, 4, 5}) --get is now {3, 4, 5}
...
buffer:mod(function(obj)
    for i=1,#obj do
        obj[i] = obj[i] + 1
    end
    return obj 
end) --is now {4, 5, 6}
...
buffer:clean() -- calls __gc early
```

### get 

returns copy of the value in the buffer 

### set 

'takes 'anything'

sets the value in the buffer 

### mod

'takes a function, one parameter, one returns

passes a copy of the value of the buffer, and sets the buffer to the value returned

```lua
buffer:mod(function(obj) return 5) 
--is the same as 
buffer:set(5)
```
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

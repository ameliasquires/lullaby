## buffer

thread.buffer(V)

a buffer is a container that allows a variable to be shared between threads & states in a thread safe manor.

it is able to store 'anything' though lightuserdata will likely not work properly. poorly structured user data may retain some shared state with the original, which could cause some use-after-free in some really poor situations. providing a __copy metamethod will alleviate this issue (read more in readme.md)

the __gc metamethod will be stripped from the original object and called when the buffer's __gc gets called. you should not reuse the original object after putting in a buffer for this reason.

the __index metamethod will index any value that is not a buffer.* method on the original object (i will try not to add any more)

every other metamethod will be replaced with a proxy to the metamethod in the copied object

inner functions can be called using the : syntactic sugar

```
buffer = llby.thread.buffer(llby.crypto.sha1())

buffer:set(buffer:get():update("awa"))
--or
buffer:set(buffer.update(buffer:get(), "awa"))

--is the same as
buffer:set(buffer:update("awa"))
```

### buffer:get

buffer:get()

copies the value in the buffer to the current state

### buffer:set

buffer:set(V)

sets the value in the buffer

### buffer:mod

buffer:mod(function(V))

takes a function with a single argument (the value), the return value of this will be the new value, if it is nil, the value will return unchanged

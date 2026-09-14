## atomic

thread.atomic(V)

a atomic is a container (also called a buffer) that allows a variable to be shared between threads & states in a thread safe manor.

it is able to store 'anything' though lightuserdata will likely not work properly. poorly structured user data may retain some shared state with the original, which could cause some use-after-free in some really poor situations. providing a __copy metamethod will alleviate this issue (read more in readme.md)

the __gc metamethod will be stripped from the original object and called when the atomic's __gc gets called. you should not reuse the original object after putting in a atomic for this reason.

the __index metamethod will index any value that is not a atomic.* method on the original object (i will try not to add any more)

every other metamethod will be replaced with a proxy to the metamethod in the copied object

inner functions can be called using the : syntactic sugar, though ownership may be needed to take into consideration

```
atomic = llby.thread.atomic(llby.crypto.sha1())

atomic:set(atomic:own():update("awa"))
--or
atomic:set(atomic.update(atomic:own(), "awa"))

--is almost the same as
atomic:set(atomic:update("awa"))
```

### atomic:get

atomic:get()

copies the value in the atomic to the current state, atomic maintains ownership (__gc)

### atomic:own

atomic:own()

the same as :get(), but the atomic transfers ownership (__gc) to the recieving state

### atomic:set

atomic:set(V)

sets the value in the atomic
returns the old value of the atomic

### atomic:mod

atomic:mod(function(V))

takes a function with a single argument (the value), the return value of this will be the new value, if it is nil, the value will return unchanged

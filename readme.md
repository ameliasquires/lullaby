build with `make`, output is `./llib.so` or (win)`./llib.dll`

windows works through msys2

[some docs](docs/)

todo:

* (working on seperatley) gui for graphs

* finish up http server

    * https

    * ~~check memory saftey~~ (*should* be good) (now work on indirect & more lifetime stuff)

    * memory optimizations (its getting there)

    * settings (parse/read files, etc..)

    * define usage of new routes inside of routes, and allow route removal

    * connection limit

    * allow choosing what to copy over to the thread, or not to copy the global state at all

    * allow stopping the server

* more doxygen like docs, everywhere

* thread-safe wrapper object

* threads

* encode tests (and fix sprintf ub)

----

# credits

* [luaproc](https://github.com/askyrme/luaproc) helped with multithreading


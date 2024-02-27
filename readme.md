build with `clang -shared src/{*.c,*/*.c} -o llib.so -fPIC`

or `clang -shared src/{*.c,*/*.c} -o llib.dll -L/mingw64/lib -llua -lws2_32 -fPIC` for my msys2 build

[some docs](docs/)

todo:

* (working on seperatley) gui for graphs

* finish up http server

    * https 

    * <res>:serve()

    * check memory saftey

    * memory optimizations

    * settings (parse/read files, etc..)

    * define usage of new routes inside of routes, and allow route removal

    * connection limit

* more doxygen like docs, everywhere

* make parray_t hash based

* str optimizations


----

# credits

* [luaproc](https://github.com/askyrme/luaproc) helped with multithreading


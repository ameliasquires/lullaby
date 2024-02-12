
build with `clang -shared src/{*.c,*/*.c} -o llib.so -fPIC`

or `clang -shared src/{*.c,*/*.c} -o llib.dll -L/mingw64/lib -llua -lws2_32 -fPIC` for my msys2 build

useage and docs coming soon:3

todo:

* (working on seperatley) gui for graphs

* fix -O3 breaking some hashes (not sure if i care)

----

credits:

* [luaproc](https://github.com/askyrme/luaproc) helped with multithreading


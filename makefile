CC := clang

GIT_COMMIT := "$(shell git describe --tags)-$(shell git describe --always --match 'NOT A TAG')"

CFLAGS := -fPIC -DGIT_COMMIT='$(GIT_COMMIT)'
LFLAGS := -lm -shared -lcrypto -lssl
LINKER := $(CC)

TARGET := lullaby.so

SRCS := $(wildcard src/*.c) $(wildcard src/*/*.c)
OBJS := $(SRCS:.c=.o)

ifeq ($(OS),Windows_NT)
	CFLAGS += -I/mingw64/include
	LFLAGS += -L/mingw64/bin -llua54 -lws2_32
	TARGET := $(TARGET:.so=.dll)
else 
	CFLAGS += -fPIC
endif

all: $(TARGET)

release: CFLAGS += -O3
release: all

# ok so im pretty sure asan should be linked too, however dlclose needs to be masked anyways
# and since libasan needs to be the first thing to load, you'll have to add it anyways
# run with something like 'LD_PRELOAD="/usr/lib/gcc/x86_64-pc-linux-gnu/14/libasan.so ./fakedlclose.so" lua5.4 ...'
# fakedlclose.so should be something as simple as the following:
# 
# "
# #include <stdio.h>
# int dlclose(void *handle) {;}
# "
#
# code (& fix) courtesy of
# https://github.com/google/sanitizers/issues/89#issuecomment-406316683
#
# this also requires lua to be built with asan
debug: CFLAGS += -ggdb3 -fno-omit-frame-pointer -fno-optimize-sibling-calls
debug: all

san: CFLAGS += -ggdb3 -static-libasan -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls
san: all

reg: 
	rm src/reg.o 

reg: all

%.o: %.c 
	$(CC) -c $< -o $@ $(CFLAGS)

$(TARGET): $(OBJS)
	$(LINKER) $(OBJS) -o $(TARGET) $(LFLAGS) 

clean: 
	rm -f $(OBJS)

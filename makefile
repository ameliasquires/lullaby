CC := clang

MAJOR_VERSION := "$(shell git -c safe.directory='*' describe --tags --abbrev=0)"
GIT_COMMIT := "$(shell git -c safe.directory='*' describe --tags --abbrev=0)-$(shell git -c safe.directory='*' describe --always --match 'NOT A TAG')"

version ?= 5.4
install_version = $(version)

ifeq ($(version),jit)
	install_version = 5.1
endif

CFLAGS := -D_GNU_SOURCE -Wall -fPIC -DGIT_COMMIT='$(GIT_COMMIT)' -DMAJOR_VERSION='$(MAJOR_VERSION)' `pkg-config --cflags lua$(version)`
LFLAGS := -lm -shared -lcrypto -lssl
LINKER := $(CC)

TARGET := lullaby.so
INSTALL := /usr/local/lib/lua/

DEBUG_CFLAGS := -ggdb3 -fno-omit-frame-pointer -fno-optimize-sibling-calls
RELEASE_FLAGS := -O3

SRCS := $(wildcard src/*.c) $(wildcard src/*/*.c)
OBJS := $(SRCS:.c=.o)

ifeq ($(OS),Windows_NT)
	CFLAGS += -I/mingw64/include
	LFLAGS += -L/mingw64/bin -llua54 -lws2_32
	TARGET := $(TARGET:.so=.dll)
else 
	CFLAGS += -fPIC
endif

.PHONY: all
all: $(TARGET)

release: CFLAGS += $(RELEASE_FLAGS)
release: all

.PHONY: install
install::
	mkdir $(INSTALL)$(install_version) -p
	cp $(TARGET) $(INSTALL)$(install_version)/$(TARGET)

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
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: all

san: CFLAGS += -static-libasan -fsanitize=address -fsanitize=undefined $(DEBUG_CFLAGS)
san: all

tsan: CFLAGS += -static-libasan -fsanitize=thread -fsanitize=undefined $(DEBUG_CFLAGS)
tsan: all

reg: 
	rm src/reg.o 

reg: all

%.o: %.c 
	$(CC) -c $< -o $@ $(CFLAGS)

$(TARGET): $(OBJS)
	$(LINKER) $(OBJS) -o $(TARGET) $(LFLAGS) 

.PHONY: clean
clean: 
	rm -f $(OBJS)



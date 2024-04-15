#todo: msys2 stuff

CC := clang
CFLAGS := -fPIC
LFLAGS := -lm -shared
LINKER := clang 

TARGET := llib.so

SRCS := $(wildcard src/*.c) $(wildcard src/*/*.c)
OBJS := $(SRCS:.c=.o)

ifeq ($(OS),Windows_NT)
	LFLAGS += -llua -lws2_32
	TARGET := $(TARGET:.so=.dll)
else ifeq ($(UNAME_S),Linux)
	#nothing
else 
	$(warning untested os, using linux defaults)
endif

all: $(TARGET)

%.o: %.c 
	$(CC) -c $< -o $@ $(CFLAGS)

$(TARGET): $(OBJS)
	$(LINKER) $(OBJS) -o $(TARGET) $(LFLAGS) 

clean: 
	rm -f $(OBJS)

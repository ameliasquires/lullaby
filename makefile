#todo: msys2 stuff

CC := clang
CFLAGS := -fPIC
LFLAGS := -lm -shared

SRCS := $(wildcard src/*.c) $(wildcard src/*/*.c)

OBJS := $(SRCS:.c=.o)

TARGET := llib.so

all: $(TARGET)

%.o: %.c 
	$(CC) -c $< -o $@ $(CFLAGS)

$(TARGET): $(OBJS)
	ld $(LFLAGS) $(OBJS) -o $(TARGET)

clean: 
	rm -f $(OBJS)

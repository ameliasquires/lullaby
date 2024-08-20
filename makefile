CC := clang

GIT_COMMIT := "$(shell git describe --tags)-$(shell git describe --always --match 'NOT A TAG')"

CFLAGS := -fPIC -DGIT_COMMIT='$(GIT_COMMIT)'
LFLAGS := -lm -shared
LINKER := clang

TARGET := lullaby.so

SRCS := $(wildcard src/*.c) $(wildcard src/*/*.c)
OBJS := $(SRCS:.c=.o)

ifeq ($(OS),Windows_NT)
	LFLAGS += -llua -lws2_32
	TARGET := $(TARGET:.so=.dll)
endif

all: $(TARGET)

debug: CFLAGS += -g
debug: all

reg: 
	rm src/reg.o 

reg: all

%.o: %.c 
	$(CC) -c $< -o $@ $(CFLAGS)

$(TARGET): $(OBJS)
	$(LINKER) $(OBJS) -o $(TARGET) $(LFLAGS) 

clean: 
	rm -f $(OBJS)

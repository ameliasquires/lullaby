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

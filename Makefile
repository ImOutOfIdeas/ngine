CC      := gcc
CFLAGS  := -g -Wall -Wextra -Iinclude
LDFLAGS := -lncursesw

SRC     := $(wildcard src/*.c)
HEADERS := $(wildcard include/*.h)
TARGET  := bin/a.out

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

run: all
	./$(TARGET)

clean:
	rm -rf bin

CC=gcc

BUILD_DIR = build
INCLUDE_DIR = include
LDLIBS = -lSDL2
CFLAGS = -Wall -Wextra
INCLFLAGS = -I $(INCLUDE_DIR)

$(BUILD_DIR)/chip8: src/main.c src/chip8.c src/input.c src/display.c
	mkdir -p $(BUILD_DIR)
	$(CC) $^ -o $@ $(CFLAGS) $(LDLIBS) $(INCLFLAGS)

clean:
	rm -rf $(BUILD_DIR)

debug: CFLAGS += -DDEBUG -g
debug: all

all: $(BUILD_DIR)/chip8
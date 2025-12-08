CC=gcc

BUILD_DIR = build
INCLUDE_DIR = include

LDLIBS = -lSDL2
CFLAGS= -I$(INCLUDE_DIR) -Wall -Wextra -g

$(BUILD_DIR)/chip8: src/main.c 
	mkdir -p $(BUILD_DIR)
	$(CC) $^ -o $@ $(CFLAGS) $(LDLIBS)

clean:
	rm -rf $(BUILD_DIR)

all: $(BUILD_DIR)/chip8
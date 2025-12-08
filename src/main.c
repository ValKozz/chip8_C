#include <stdio.h>

#include "../include/chip8.h"

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("Use: ch8 <rom-file>\n");
		return 1;
	}

	// create it on the stack
	chip8_t chip8;

	if (init(&chip8, argv[1]) == 1) {
		return 1;
	}

	while (chip8.running_flag) {
		decode_and_exec(&chip8);
		fetch(&chip8);
	}
}
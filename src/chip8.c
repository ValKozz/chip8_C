#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>

#include "../include/chip8.h"
#include "SDL2/SDL.h"

#define PC_START 0x200
#define MEM_END  0xFFF

uint8_t fonts[] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

int init(chip8_t *chip8, char *rom_path) {
	if (chip8 == NULL || rom_path == NULL) {
		return 1;
	}
	// init the display
	display_t displ = displ_init();
	if (displ == NULL) return 1;

	chip8->displ = displ;
	// zero out the memory
	memset(chip8->memory, 0, sizeof(SYS_MEMORY));
	// copy the fonts into memory
	memcpy((chip8->memory), fonts, sizeof(fonts));

	FILE *fp = fopen(rom_path, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Error, opening ch8 image (NULL): %s\n", rom_path);
		return 1;
	}

	// Get ROM len
	fseek(fp, 0, SEEK_END);
	size_t rom_len = ftell(fp);
	rewind(fp);

	if ((MEM_END - PC_START) < rom_len) {
		fprintf(stderr, "Error, image too large!\n");
		fclose(fp);
		return 1;
	}
	// Load from 0x200 forward
	fread(chip8->memory + PC_START, sizeof(uint8_t), rom_len, fp);
	chip8->stack.size = 0;

	// put the PC at 0x200
	chip8->PC = PC_START;

	chip8->opcode = 
		(chip8->memory[chip8->PC] << 8) + chip8->memory[chip8->PC+1];

	chip8->DT = 0;
	chip8->ST = 0;
	chip8->I = 0;

	chip8->key = 0;

	chip8->running = 1;

	fclose(fp);
	return 0;

}

void fetch(chip8_t *chip8) {
	if (!chip8->paused)
		chip8->PC += 2;
	// Reverse endian
	chip8->opcode = 
		(chip8->memory[chip8->PC] << 8) + chip8->memory[chip8->PC+1];
}

void decode_and_exec(chip8_t *chip8) {
	// if not running, free the display and destroy SDL
	if (!chip8->running) {
		displ_destroy(chip8->displ);
		return;
	}

	if (chip8->paused) return;
	uint16_t instr = chip8->opcode;
	uint8_t flag = (chip8->opcode >> 8) & 0xF0;

	#ifdef DEBUG
	printf("OPCODE: %04x\n", instr);
	#endif

	// TODO
	// Parse instructions
	switch (flag) {
	case 0x00:
		if (instr == 0x00E0) {
			#ifdef DEBUG
			printf("Clear screen.\n");
			#endif
			displ_clear(chip8->displ);
		}
		else if (instr == 0x00EE) {			
			if (chip8->stack.size > 0) {
				#ifdef DEBUG
				printf("Return to addr %d from stack\n", *(chip8->stack.array[chip8->stack.size]));
				#endif
				// very ugly, TODO
				chip8->PC = *(chip8->stack.array[chip8->stack.size]);
				chip8->stack.size--;
			}
			else {
				fprintf(stderr, "Stack underflow Error!\n");
				chip8->running = 0;
				break;
			}
		}
		else {
			// TODO 
			uint8_t addr = instr & 0x0FFF;
			
			#ifdef DEBUG
			printf("Call machine code to %d.\n", 
				addr);
			#endif
			
			if (chip8->stack.size >= 16) {
				fprintf(stderr, "Stack overflow Error!\n");
				chip8->running = 0;
				break;
			}

			*(chip8->stack.array[chip8->stack.size++]) = chip8->PC;
			chip8->PC = addr;
			chip8->opcode = (chip8->memory[chip8->PC] << 8) + chip8->memory[chip8->PC+1];
		}
		break;
	default:
		// fprintf(stderr, "UNKNOWN OPCODE: %04x\n", instr);
		// chip8->running = 0;
		break;		
	}

	if (chip8->PC+1 == MEM_END) {
		printf("Reached MEM_END\n");
		// temp
		chip8->paused = 1;
	}

}

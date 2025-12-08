#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>

#include "../include/chip8.h"

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
	chip8->PC = (uint16_t *)(chip8->memory + PC_START);
	chip8->DT = 0;
	chip8->ST = 0;
	chip8->I = 0;

	chip8->running_flag = 1;

	fclose(fp);
	return 0;

}

void fetch(chip8_t *chip8) {
	chip8->PC++;
}

void decode_and_exec(chip8_t *chip8) {
	// TODO
	uint16_t instr = *(chip8->PC);
	printf("INSTR: %04x\n", instr);

	if ((uint8_t *)chip8->PC == chip8->memory + MEM_END) {
		printf("Reached MEM_END\n");
		chip8->running_flag = 0;
	}

	// if not running, free the display
	if (!chip8->running_flag) displ_destroy(chip8->displ);
}
#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>

#include "../include/chip8.h"

#define PC_START 0x200
#define MEM_END  0xFFF

int init(chip8_t *chip8, char *rom_path) {
	if (chip8 == NULL || rom_path == NULL) {
		return 1;
	}

	display_t displ = displ_init();
	if (displ == NULL) return 1;

	chip8->displ = displ;

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

	if (chip8->PC == (uint16_t *)chip8->memory + MEM_END) {
		printf("Reached MEM_END\n");
		chip8->running_flag = 0;
	}

	// if not running, free the display
	if (!chip8->running_flag) displ_destroy(chip8->displ);
}
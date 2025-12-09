#ifndef _CHIP8_H_
#define _CHIP8_H_

#include <stddef.h>
#include <inttypes.h>

#include "../include/display.h"

#define SYS_MEMORY 4096

typedef struct stack {
	size_t size;
	uint16_t *array[16];
} stack_t;

typedef struct chip8 {
	uint8_t memory[SYS_MEMORY];
	stack_t	stack;
	uint8_t registers[16];

	uint16_t opcode; // current opcode
	uint16_t PC;
	uint16_t I;
	uint8_t SP;
	uint8_t DT;
	uint8_t ST;

	uint8_t key;	// current pressed key, 0 means None
	// display struct pointer used to handle drawing to an SDL window
	display_t displ;
	int running			:1; // flag
	int draw			:1;	// flag
	int paused			:1; // flag
	int jmp_flag		:1;
} chip8_t; 

enum registers {
	V0 = 0,
	V1,
	V2, 
	V3,
	V4,
	V5,
	V6,
	V7,
	V8,
	V9,
	VA,
	VB,
	VC,
	VD,
	VE,
	VF	// used as flag and carry bit
};

// The chip8 struct will be created on the stack in main, to skip uneccessery freeing and allocation
int init(chip8_t *chip8, char *rom_path);
// increment PC
void fetch(chip8_t *chip8);
// decode and execute instruction
void decode_and_exec(chip8_t *chip8);
#endif
#ifndef _CHIP8_H_
#define _CHIP8_H_

#include <stddef.h>
#include <inttypes.h>

#include "SDL2/SDL.h"

#define SYS_MEMORY 4096
#define DISPLAY_WIDTH  64
#define DISPLAY_HEIGHT 32

typedef struct stack {
	size_t size;
	uint16_t array[16];
} stack_t;

typedef struct chip8 {
	uint8_t memory[SYS_MEMORY];
	// screen buffer used to hold the pixels of the display
	uint8_t screen[DISPLAY_WIDTH][DISPLAY_HEIGHT];
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	
	stack_t	stack;
	uint8_t registers[16];

	uint16_t opcode; // current opcode, uint16_t union
	uint16_t PC;
	uint16_t I;
	uint8_t SP;
	uint8_t DT;
	uint8_t ST;

	uint8_t key;	// current pressed key, 0 means None
	// display struct pointer used to handle drawing to an SDL window
	uint8_t running			:1; // flag
	uint8_t draw			:1;	// flag
	uint8_t paused			:1; // flag
	uint8_t jmp_flag		:1;
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
// increment PC and store instruction
void fetch(chip8_t *chip8);
// decode and execute instruction
void decode_and_exec(chip8_t *chip8);
#endif
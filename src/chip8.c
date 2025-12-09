#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

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

static void store_instr(chip8_t *chip8);
// push to stack and do error checking
static void push_stack(chip8_t *chip8);
// pops the last address from the stack and stores it in PC
static void pop_stack(chip8_t *chip8);

static void store_instr(chip8_t *chip8) {
	// Reverse endian
	chip8->opcode = 
		(chip8->memory[chip8->PC] << 8) + chip8->memory[chip8->PC+1];
}

static void push_stack(chip8_t *chip8) {
	if (chip8->stack.size >= 15) {
		fprintf(stderr, "Stack overflow Error!\n");
		chip8->running = 0;
		return;
		}

	chip8->stack.array[chip8->stack.size] = chip8->PC;
	chip8->stack.size++;
	#ifdef DEBUG
	printf("Pushed to stack: %d\n", chip8->PC);
	#endif
}

static void pop_stack(chip8_t *chip8) {
	if (chip8->stack.size == 0) {
		fprintf(stderr, "Stack underflow Error!\n");
		chip8->running = 0;
		return;
	}
	chip8->PC = chip8->stack.array[chip8->stack.size-1];
	chip8->stack.size--;
	#ifdef DEBUG
	printf("Poppef from stack: %d\n", chip8->PC);
	#endif
}

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
	store_instr(chip8);

	chip8->DT = 0;
	chip8->ST = 0;
	chip8->I = 0;

	chip8->key = 0;

	chip8->running = 1;

	fclose(fp);
	return 0;

}

void fetch(chip8_t *chip8) {
	if (!chip8->paused && !chip8->jmp_flag) {
		chip8->PC += 2;
	} 
	else chip8->jmp_flag = 0; // else set jmp to 0

	store_instr(chip8);
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
	printf("OPCODE: %04x ", instr);
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
			#ifdef DEBUG
			printf("Return to addr %d from stack\n", chip8->stack.array[chip8->stack.size]);
			#endif
			pop_stack(chip8);
			}

		else {
			uint16_t NNN = instr & 0x0FFF;
			
			#ifdef DEBUG
			printf("Call machine code to %d.\n", NNN);
			#endif
			// store address on the stack 
			push_stack(chip8);
			chip8->PC = NNN;
			chip8->jmp_flag = 1;
		}
		break;
	case 0x10: {
		uint16_t NNN = instr & 0x0FFF;
		#ifdef DEBUG
		printf("JMP to %d.\n", NNN);
		#endif

		chip8->jmp_flag = 1;
		
		if (NNN > SYS_MEMORY) {
			fprintf(stderr, "ERROR: JMP out of memory! %d\n", NNN);
			chip8->running = 0;
			return;
		}

		chip8->PC = NNN;
		break;
	}
	case 0x20: {
		uint16_t NNN = instr & 0x0FFF;

		if (NNN > SYS_MEMORY) {
			fprintf(stderr, "ERROR: JMP to subroutine out of memory! %d\n", NNN);
			chip8->running = 0;
			return;
		}

		#ifdef DEBUG
		printf("Call subroutine at: %d\n", NNN);
		#endif

		push_stack(chip8);
		chip8->PC = NNN;
		chip8->jmp_flag = 1;
		break;	
	}
case 0x30: {
	uint8_t vx = (instr & 0x0F00) >> 8;
	uint8_t NN = instr & 0x00FF;
	
	if (vx > VF) {
		fprintf(stderr, "ERROR: Tried to access non-existant register NNN: %d", vx);
		chip8->running = 0;
		return;
	}

	#ifdef DEBUG
	printf("Compare: %d == %d(NN)\n", 
		chip8->registers[vx], NN);
	#endif
	// Skips the next instruction if VX equals NN
	if (chip8->registers[vx] == NN) {
		chip8->PC += 2;
	}
	break;
	}
case 0x40: {
	uint8_t vx = (instr & 0x0F00) >> 8;
	uint8_t NN = instr & 0x00FF;

	if (vx > VF) {
		fprintf(stderr, "ERROR: Tried to access non-existant register NNN: %d", vx);
		chip8->running = 0;
		return;
	}
	#ifdef DEBUG
	printf("Compare NOT: %d != %d(NN)\n", 
		chip8->registers[vx], NN);
	#endif

	// Skip next instruction if NN != Vx
	if (chip8->registers[vx] != NN) {
		chip8->PC += 2;
	}  
	break;
	}
case 0x50: {
	uint8_t vx = (instr & 0x0F00) >> 8;
	uint8_t vy = (instr & 0X00F0) >> 4;
	
	if (vx > VF || vy > VF) {
		fprintf(stderr, "ERROR: Tried to access non-existant register NNN: vx: %d vy: %d", vx, vy);
		chip8->running = 0;
		return;
	}
	#ifdef DEBUG
	printf("Compare: %d == %d(vy)\n", 
		chip8->registers[vx], chip8->registers[vy]);
	#endif

	if (chip8->registers[vx] == chip8->registers[vy]) {
		chip8->PC += 2;
	}

	break;
	}
case 0x60: {
	uint8_t vx = (instr & 0x0F00) >> 8;
	uint8_t NN = instr & 0x00FF;

	if (vx > VF) {
		fprintf(stderr, "ERROR: Tried to access non-existant register NNN: %d", vx);
		chip8->running = 0;
		return;
	}
	#ifdef DEBUG
	printf("V%d set to %d == %d(NN)\n", 
		vx, chip8->registers[vx], NN);
	#endif
	chip8->registers[vx] = NN;
	break;
	}
case 0x70: {
	uint8_t vx = (instr & 0x0F00) >> 8;
	uint8_t NN = instr & 0x00FF;

	if (vx > VF) {
		fprintf(stderr, "ERROR: Tried to access non-existant register NNN: %d", vx);
		chip8->running = 0;
		return;
	}
	#ifdef DEBUG
	printf("Add %d to V%d = %d\n", 
		NN, vx, chip8->registers[vx]);
	#endif
	chip8->registers[vx] += NN; 
	break;
	}
case 0x80: {
	uint8_t subflag = instr & 0x000F;
	uint8_t vx = (instr & 0x0F00) >> 8;
	uint8_t vy = (instr & 0X00F0) >> 4;

	if (vx > VF || vy > VF) {
		fprintf(stderr, "ERROR: Tried to access non-existant register NNN: vx: %d vy: %d", vx, vy);
		chip8->running = 0;
		return;
	}

	switch (subflag) {
		case 0x0:
			chip8->registers[vx] = chip8->registers[vy];
			#ifdef DEBUG
			printf("Set V%d to V%d = %d\n", 
			vx, vy, chip8->registers[vx]);
			#endif
			break;
		case 0x1:
			chip8->registers[vx] |= chip8->registers[vy];
			#ifdef DEBUG
			printf("bitwise OR V%d to V%d = %d\n", 
			vx, vy, chip8->registers[vx]);
			#endif
			break;
		case 0x2:
			chip8->registers[vx] &= chip8->registers[vy];
			#ifdef DEBUG
			printf("bitwise AND V%d to V%d = %d\n", 
			vx, vy, chip8->registers[vx]);
			#endif
			break;
		case 0x3:
			chip8->registers[vx] ^= chip8->registers[vy];
			#ifdef DEBUG
			printf("bitwise XOR V%d to V%d = %d\n", 
			vx, vy, chip8->registers[vx]);
			#endif
			break;
		case 0x4: {
			uint16_t res = chip8->registers[vx] + chip8->registers[vy];
			// max value of a uint8_t 255
			// check for overflow first
			if (res > UINT8_MAX) {
				#ifdef DEBUG
				printf("overflow detected:  ");
				#endif		
				chip8->registers[VF] = 1;
			}
			else chip8->registers[VF] = 0;
			
			chip8->registers[vx] += chip8->registers[vy];
			
			#ifdef DEBUG
			printf("Add V%d to V%d = %d\n", 
			vy, vx, chip8->registers[vx]);
			#endif
			break;
			}
		case 0x5: {
			// signed to check for underflow
			int res = chip8->registers[vx] - chip8->registers[vy];
			// max value of a uint8_t 255
			// check for overflow first
			if (res < 0) {
				#ifdef DEBUG
				printf("underflow detected: ");
				#endif		
				chip8->registers[VF] = 0;
			}
			else chip8->registers[VF] = 1;
			
			chip8->registers[vx] -= chip8->registers[vy];
			
			#ifdef DEBUG
			printf("substract V%d to V%d = %d\n", 
			vy, vx, chip8->registers[vx]);
			#endif
			break;
		}
		case 0x6: {
			// store least significant bit in VF
			chip8->registers[VF] = chip8->registers[vx] & 0x1;
			chip8->registers[vx] >>= 1;

			#ifndef DEBUG
			printf("Shift V%d >> 1 = %d\n",
				vx, chip8->registers[vx]);
			#endif
			break;
		}
		case 0x7: {
			int res = chip8->registers[vy] - chip8->registers[vx];
			if (res < 0) {
				#ifdef DEBUG
				printf("underflow detected: ");
				#endif
				chip8->registers[VF] = 0;
			}
			else chip8->registers[VF] = 1;
			chip8->registers[vx] = chip8->registers[vy] - chip8->registers[vx];
			#ifdef DEBUG
			printf("substract V%d(Vy) - V%d = %d\n",
				vy, vx, chip8->registers[vx]);
			#endif
			break;
		}
		case 0xE: {
			// store most significant
			chip8->registers[VF] = chip8->registers[vx] & (0x01 << 7);
			chip8->registers[vx] <<= 1;
			#ifndef DEBUG
			printf("Shift V%d << 1 = %d\n",
				vx, chip8->registers[vx]);
			#endif
			break;
		}
	}
	break;
	}

case 0x90: {
	uint8_t vx = (instr & 0x0F00) >> 8;
	uint8_t vy = (instr & 0X00F0) >> 4;
		
	if (vx > VF || vy > VF) {
		fprintf(stderr, "ERROR: Tried to access non-existant register NNN: vx: %d vy: %d", vx, vy);
		chip8->running = 0;
		return;
	}

	#ifndef DEBUG
	printf("Skip next V%d != V%d\n",
		vx, vy);
	#endif

	if (chip8->registers[vx] != chip8->registers[vy]) {
		chip8->PC +=2;
	}
	break;	
	}
case 0xA0: {
	uint16_t NNN = instr & 0X0FFF;
	chip8->I = NNN;
	#ifndef DEBUG
	printf("Set I to %d\n",
		chip8->I);
	#endif
	break;
	}
case 0xB0: {
	uint16_t NNN = instr & 0X0FFF;
	#ifndef DEBUG
	printf("JMP to (V0) %d + %d = %d\n",
		chip8->registers[V0], NNN, NNN + chip8->registers[V0]);
	#endif
	
	chip8->PC = chip8->registers[V0] + NNN;
	chip8->jmp_flag = 1;
	break;
	}
case 0xD0: {
	// TODO draw
	uint8_t vx = (instr & 0x0F00) >> 8;
	uint8_t vy = (instr & 0X00F0) >> 4;
	uint8_t N = instr & 0X000F;

	if (vx > VF || vy > VF) {
		fprintf(stderr, "ERROR: Tried to access non-existant register NNN: vx: %d vy: %d", vx, vy);
		chip8->running = 0;
		return;
	}

	#ifdef DEBUG
	printf("TODO Draw on screen: %d %d height: %d\n",
		chip8->registers[vx], chip8->registers[vy], N);
	#endif

	break;
	}
case 0xE0: {
	uint16_t subflag = instr & 0x00FF;
	if (subflag == 0x9E) {

	}	
	else if (subflag == 0xA1) {

	}
	else {
		fprintf(stderr, "ERROR: ILLEGAL OPCODE 0xE0: %04x\n", instr);
		chip8->running = 0;
		return;
		}
	}
	break;

	default:
		fprintf(stderr, "ERROR: UNKNOWN OPCODE: %04x\n", instr);
		chip8->running = 0;
		return;
	}

	if (chip8->PC+1 == MEM_END) {
		fprintf(stderr, "ERROR: Reached MEM_END\n");
		// temp
		chip8->paused = 1;
	}

}

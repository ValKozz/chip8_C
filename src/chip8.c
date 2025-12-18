#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include "../include/chip8.h"
#include "../include/input.h"
#include "../include/display.h"

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

// validation
static void validate_NNN(chip8_t *chip8, uint16_t NNN);
static void validate_X(chip8_t *chip8, uint8_t X);
static void validate_XY(chip8_t *chip8, uint8_t X, uint8_t Y);

static void store_instr(chip8_t *chip8) {
	// Reverse endian, store in union's largest value
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
	printf("Popped from stack: %d\n", chip8->PC);
	#endif
}

int init(chip8_t *chip8, char *rom_path) {
	if (chip8 == NULL || rom_path == NULL) {
		return 1;
	}

	if (displ_init_SDL()) return 1;
	chip8->window = displ_init_Window();
	chip8->renderer = displ_init_Renderer(chip8->window);
	chip8->texture = displ_init_Texture(chip8->renderer);

	if (chip8->window == NULL || chip8->renderer == NULL || chip8->texture == NULL){
		displ_destroy(chip8);
		return 1;	
	} 

	// apply scale 10 less, so 1 pixel equals 10
	if (SDL_RenderSetScale(chip8->renderer, 10.0f, 10.0f) < 0) {
		SDL_DestroyWindow(chip8->window);
		SDL_DestroyRenderer(chip8->renderer);

		fprintf(stderr, "Error setting window scale! %s\n", SDL_GetError());
		return 1;
	}

	displ_clear(chip8);

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
	#ifdef DEBUG
	printf("JMP FLAG: %d ", chip8->jmp_flag);
	printf("PC = %2x %d\n", chip8->PC, chip8->PC);
	#endif

	store_instr(chip8);

	if (chip8->jmp_flag == 0 && chip8->paused == 0) {
		chip8->PC += 2;
	} 	

	chip8->jmp_flag = 0; // set jmp to 0 if jumped

	#ifdef DEBUG
	printf("STORED NEXT INSTR %04x\n",chip8->opcode);
	#endif
}

void decode_and_exec(chip8_t *chip8) {
	// if not running, free the display and destroy SDL
	if (!chip8->running) {
		displ_destroy(chip8);
		return;
	}
	if (chip8->paused) return;

	uint8_t flag = (chip8->opcode & 0xF000) >> 12;
	uint8_t X = (chip8->opcode & 0x0F00) >> 8;
	uint8_t Y = (chip8->opcode & 0x00F0) >> 4;;
	uint8_t N = chip8->opcode & 0x000F;

	uint8_t NN = chip8->opcode & 0x00FF;
	uint16_t NNN = chip8->opcode & 0X0FFF;

	#ifdef DEBUG
	printf("OPCODE: %04x ", chip8->opcode);
	#endif

	// Parse instructions
	switch (flag) {
	case 0x0:
		if (chip8->opcode == 0x00E0) {
			#ifdef DEBUG
			printf("Clear screen.\n");
			#endif
			displ_clear(chip8);
		}

		else if (chip8->opcode == 0x00EE) {			
			#ifdef DEBUG
			printf("Return to addr %d from stack\n", chip8->stack.array[chip8->stack.size - 1]);
			#endif
			pop_stack(chip8);
			}

		else {			
			#ifdef DEBUG
			printf("Call machine code to %d.\n", NNN);
			#endif
			// store address on the stack 
			push_stack(chip8);
			chip8->PC = NNN;
			chip8->jmp_flag = 1;
		}
		break;
	case 0x1: {
		#ifdef DEBUG
		printf("JMP to %d. ", NNN);
		#endif
		
		validate_NNN(chip8, NNN);

		chip8->jmp_flag = 1;
		chip8->PC = NNN;
		break;
	}
	case 0x2: {
		validate_NNN(chip8, NNN);

		#ifdef DEBUG
		printf("Call subroutine at: %d\n", NNN);
		#endif

		push_stack(chip8);
		chip8->PC = NNN;
		chip8->jmp_flag = 1;
		break;	
	}
	case 0x3: {
		validate_X(chip8, X);

		#ifdef DEBUG
		printf("Compare: %d == %d (NN) ", 
			chip8->registers[X], NN);
		if (chip8->registers[X] == NN) printf("Skipped\n");
		else printf("\n");
		#endif
		// Skips the next instruction if VX equals NN
		if (chip8->registers[X] == NN) {
			chip8->PC += 2;
		}
		break;
		}
	case 0x4: {
		validate_X(chip8, X);

		#ifdef DEBUG
		printf("Compare NOT: %d != %d(NN) ", 
			chip8->registers[X], NN);
		if (chip8->registers[X] != NN) printf("Skipped\n");
		else printf("\n");
		#endif

		// Skip next instruction if NN != Vx
		if (chip8->registers[X] != NN) {
			chip8->PC += 2;
		}  
		break;
		}
	case 0x5: {
		validate_XY(chip8, X, Y);

		#ifdef DEBUG
		printf("Compare: %d == %d(Y) ", 
			chip8->registers[X], chip8->registers[Y]);
		if (chip8->registers[X] == chip8->registers[Y]) printf("Skipped\n");
		else printf("\n");
		#endif

		if (chip8->registers[X] == chip8->registers[Y]) {
			chip8->PC += 2;
		}

		break;
		}
	case 0x6: {
		validate_X(chip8, X);

		#ifdef DEBUG
		printf("V%d set to %d\n", 
			X, chip8->registers[X]);
		#endif
		chip8->registers[X] = NN;
		break;
		}
	case 0x7: {
		validate_X(chip8, X);

		#ifdef DEBUG
		printf("Add %d to V%d = %d\n", 
			NN, X, chip8->registers[X]);
		#endif
		chip8->registers[X] += NN; 
		break;
		}
	case 0x8: {
		validate_XY(chip8, X, Y);

		switch (N) {
			case 0x0:
				chip8->registers[X] = chip8->registers[Y];
				#ifdef DEBUG
				printf("Set V%d to V%d = %d\n", 
				X, Y, chip8->registers[X]);
				#endif
				break;
			case 0x1:
				chip8->registers[X] |= chip8->registers[Y];
				#ifdef DEBUG
				printf("bitwise OR V%d to V%d = %d\n", 
				X, Y, chip8->registers[X]);
				#endif
				break;
			case 0x2:
				chip8->registers[X] &= chip8->registers[Y];
				#ifdef DEBUG
				printf("bitwise AND V%d to V%d = %d\n", 
				X, Y, chip8->registers[X]);
				#endif
				break;
			case 0x3:
				chip8->registers[X] &= chip8->registers[Y];
				#ifdef DEBUG
				printf("bitwise XOR V%d to V%d = %d\n", 
				X, Y, chip8->registers[X]);
				#endif
				break;
			case 0x4: {
				uint16_t res = chip8->registers[X] + chip8->registers[Y];
				// max value of a uint8_t 255
				// check for overflow first
				if (res > UINT8_MAX) {
					#ifdef DEBUG
					printf("overflow detected:  ");
					#endif		
					chip8->registers[VF] = 1;
				}
				else chip8->registers[VF] = 0;
				
				chip8->registers[X] += chip8->registers[Y];
				
				#ifdef DEBUG
				printf("Add V%d to V%d(X) = %d\n", 
				Y, X, chip8->registers[X]);
				#endif
				break;
				}
			case 0x5: {
				// signed to check for underflow
				int res = chip8->registers[X] - chip8->registers[Y];
				// max value of a uint8_t 255
				// check for overflow first
				if (res < 0) {
					#ifdef DEBUG
					printf("underflow detected: ");
					#endif		
					chip8->registers[VF] = 0;
				}
				else chip8->registers[VF] = 1;
				
				chip8->registers[X] -= chip8->registers[Y];
				
				#ifdef DEBUG
				printf("substract V%d from V%d(X) = %d\n", 
				Y, X, chip8->registers[X]);
				#endif
				break;
			}
			case 0x6: {
				// store least significant bit in VF
				chip8->registers[VF] = chip8->registers[X] & 0x1;
				chip8->registers[X] >>= 1;

				#ifdef DEBUG
				printf("Shift V%d >> 1 = %d\n",
					X, chip8->registers[X]);
				#endif
				break;
			}
			case 0x7: {
				int res = chip8->registers[Y] - chip8->registers[X];
				if (res < 0) {
					#ifdef DEBUG
					printf("underflow detected: ");
					#endif
					chip8->registers[VF] = 0;
				}
				else chip8->registers[VF] = 1;
				chip8->registers[X] = chip8->registers[Y] - chip8->registers[X];
				#ifdef DEBUG
				printf("substract V%d(Y) - V%d = %d\n",
					Y, X, chip8->registers[X]);
				#endif
				break;
			}
			case 0x0E: {
				// store most significant
				chip8->registers[VF] = chip8->registers[X] & (0x01 << 7);
				chip8->registers[X] <<= 1;
				#ifdef DEBUG
				printf("Shift V%d << 1 = %d\n",
					X, chip8->registers[X]);
				#endif
				break;
			}
		}
		break;
		}

	case 0x9: {
		validate_XY(chip8, X, Y);
		
		#ifdef DEBUG
		printf("Skip next V%d != V%d\n",
			X, Y);
		#endif

		if (chip8->registers[X] != chip8->registers[Y]) {
			chip8->PC +=2;
		}
		break;	
		}
	case 0xA: {
		validate_NNN(chip8, NNN);

		chip8->I = NNN;
		#ifdef DEBUG
		printf("Set I to %d\n",
			chip8->I);
		#endif
		break;
		}
	case 0xB: {
		validate_NNN(chip8, NNN);

		#ifdef DEBUG
		printf("JMP to (V0) %d + %d = %d\n",
			chip8->registers[V0], NNN, NNN + chip8->registers[V0]);
		#endif
		
		chip8->PC = chip8->registers[V0] + NNN;
		chip8->jmp_flag = 1;
		break;
		}
	case 0xD: {
		validate_XY(chip8, X, Y);
		// TODO draw
		// reset the register
		chip8->registers[VF] = 0;

		uint8_t screen_x = chip8->registers[X];
		uint8_t screen_y = chip8->registers[Y];

		#ifdef DEBUG
		printf("Draw on screen: %d(V%d) %d(V%d) height: %d\n",
		screen_x, X, screen_y, Y, N);
		#endif

		for (uint8_t yc = 0; yc < N; yc++) {
			// reverse the byte order
			uint8_t sprite_byte = 
				chip8->memory[chip8->I + yc];
			for (uint8_t xc = 0; xc < 8; xc++) {
				printf("%d ", (sprite_byte >> (7 - xc)) & 0x1);
				if ((sprite_byte >> (7 - xc)) & 0x1) {
					// loop around if bigger 
					uint8_t x = (screen_x + xc) % DISPLAY_WIDTH;
					uint8_t y = (screen_y + yc) % DISPLAY_HEIGHT;
					// collision
					if (chip8->screen[x][y]) chip8->registers[VF] = 1;
					// XOR the pixel
					chip8->screen[x][y] ^= 1;
				}
			}
			printf("\n");
		}

		chip8->draw = 1;
		break;
		}
	case 0xE: {
		validate_X(chip8, X);

		if (NN == 0x9E) {
			#ifdef DEBUG
				printf("Skip on key %d\n", chip8->registers[X]);
			#endif

			if (chip8->key == chip8->registers[X]) {
				chip8->PC += 2;
			}
		}	
		else if (NN == 0xA1) {
			#ifdef DEBUG
			printf("Skip NOT on key %d\n", chip8->registers[X]);
			#endif

			if (chip8->key != chip8->registers[X]) {
				chip8->PC += 2;
			}
		else {
			fprintf(stderr, "ERROR: ILLEGAL OPCODE in 0xE0: %04x\n", chip8->opcode);
			chip8->running = 0;
			return;
			}
		}
		break;
		}
	case 0xF: {
		switch (NN) {
		case 0x07:
			#ifdef DEBUG
			printf("Set V%d to timer %d\n", X, chip8->DT);
			#endif
			chip8->registers[X] = chip8->DT;
			break;
		case 0x0A:
			// TODO ST and DT should continue
			// A halt flag should be implemented?
			#ifdef DEBUG
			printf("Await input to set V%d to\n", X);
			#endif

			while (chip8->key == 0) {
				handle_input(chip8);
			}

			chip8->registers[X] = chip8->key;
			break;
		case 0x15:
			#ifdef DEBUG
			printf("Set DT to %d\n", chip8->registers[X]);
			#endif
			chip8->DT = chip8->registers[X];
			break;
		case 0x18:
			#ifdef DEBUG
			printf("Set ST to %d\n", chip8->registers[X]);
			#endif
			chip8->ST = chip8->registers[X];
			break;
		case 0x1E:
			#ifdef DEBUG
			printf("Add %d to I = %d\n", chip8->registers[X], chip8->I + chip8->registers[X]);
			#endif
			chip8->I += chip8->registers[X];
			break;
		case 0x29:
			#ifdef DEBUG
			printf("Set I sprite addr: %d\n", chip8->registers[X]);
			#endif
			chip8->I = chip8->registers[X] * 0x5;
			break;
		case 0x33:
			#ifdef DEBUG
			printf("set BCD OP\n");
			#endif
			chip8->memory[chip8->I] = chip8->registers[X] / 100;
			chip8->memory[chip8->I + 1] = (chip8->registers[X] / 10) % 10;
			chip8->memory[chip8->I + 2] = (chip8->registers[X] % 100) % 10;
			break;
		case 0x55: {
			size_t i = 0;
			while (i < VF) {
				chip8->memory[chip8->I + i] = chip8->registers[i];
				i++;
			}
			break;
		}
		case 0x65: {
			size_t i = 0;
			while (i < VF) {
				chip8->registers[i] = chip8->memory[chip8->I + i];
				i++;
			}
			break;
		}
		default:
			fprintf(stderr, "ERROR: ILLEGAL OPCODE in 0xF0: %04x\n", chip8->opcode);
			chip8->running = 0;
			return;
		}
		break;
	}

	default:
		fprintf(stderr, "ERROR: UNKNOWN OPCODE: %04x\n", chip8->opcode);
		chip8->running = 0;
		return;
	}
	// check if screen needs to be updated
	if (chip8->draw == 1) {
		displ_present(chip8);
		chip8->draw = 0;
	}

	if (chip8->PC+2 == MEM_END) {
		fprintf(stderr, "ERROR: PC Reached MEM_END\n");
		// temp
		chip8->paused = 1;
	}
}

static void validate_NNN(chip8_t *chip8, uint16_t NNN) {
	if (NNN > 0xFFF) {
		fprintf(stderr, "Attempted to reach address outside RAM! %d\n", NNN);
		chip8->running = 0;
	}
}

static void validate_X(chip8_t *chip8, uint8_t X) {
	if (X > VF) {
		fprintf(stderr, "ERROR: Tried to access non-existant register: %d", X);
		chip8->running = 0;
	}
}

static void validate_XY(chip8_t *chip8, uint8_t X, uint8_t Y) {
	if (X > VF || Y > VF) {
		fprintf(stderr, "ERROR: Tried to access non-existant register: %d(X) %d(Y)", X, Y);
		chip8->running = 0;
	}
}


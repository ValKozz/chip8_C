#ifndef _INPUT_H_
#define _INPUT_H_

#include "../include/chip8.h"

/* Used to handle user input, keys are 
	
	1 2 3 C -> 1 2 3 4
	4 5 6 D	-> Q W E R
	7 8 9 E -> A S D F
	A 0 B F -> Z X C V

	Ressembling the original Hex keyboard layout used.
*/
enum keys {
	K_1 = 1,
	K_2,
	K_3,
	K_4,
	K_Q,
	K_W,
	K_E,
	K_R,
	K_A,
	K_S,
	K_D,
	K_F,
	K_Z,
	K_X,
	K_C,
	K_V
};

// Used for user input
void handle_input(chip8_t *chip8);
#endif
#include <stdbool.h>
#include <stdio.h>

#include "../include/instruction.h"
#include "../include/chip8.h"

bool validate_NNN(instruction instr) {
	if (instr.NNN.value > 0xFFF) {
		fprintf(stderr, "Attempted to reach address outside RAM! %d\n", instr.NNN.value)
		return false;
	}
	return true;
}

bool validate_X(instruction instr) {
	if (instr.flags.X > VF) {
		fprintf(stderr, "ERROR: Tried to access non-existant register: %d", vx);
		return false;
	}
	return true;
}

bool validate_XY(instruction instr) {
	if (instr.flags.X > VF || instr.flags.Y > VF) {
		fprintf(stderr, "ERROR: Tried to access non-existant register: %d", vx);
		return false;
	}
	return true;
}

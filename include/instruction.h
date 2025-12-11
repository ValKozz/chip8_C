#ifndef _INSTRUCTION_H_
#define _INSTRUCTION_H_

#include <stdint.h>
#include <stdbool.h>

struct instr_flags {
	uint8_t flag 	:4;
	uint8_t X 		:4;
	uint8_t Y 		:4;
	uint8_t N       :4;
};

struct _NN {
	uint8_t 		: 8;
	uint8_t value	: 8;
};

struct _NNN {
	uint8_t 		:4;
	uint16_t value 	:12;
};
// instruction union to make it easier to work with
// and remove the need to do bitwise AND to get the values
typedef union instruction {
	uint16_t body;
	struct instr_flags flags;
	struct _NN NN;
	struct _NNN NNN;
} instruction;

bool validate_NNN(instruction instr);
bool validate_X(instruction instr);
bool validate_XY(instruction instr);

#endif
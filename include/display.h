#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <inttypes.h>
#include "SDL2/SDL.h"

#define DISPLAY_HEIGHT 32
#define DISPLAY_WIDTH  64

typedef struct display {
	// used to hold the pixels of the display
	uint8_t screen[DISPLAY_HEIGHT][DISPLAY_WIDTH];
	SDL_Window *window;
	SDL_Renderer *renderer;
} *display_t;

// Init SDL, window and renderer and return the struct holding them
display_t displ_init(void);
void displ_destroy(display_t displ);

#endif
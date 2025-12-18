#ifndef _DISPLAY_H_
#define _DISPLAY_H_
	
#include "chip8.h"

int displ_init_SDL();
SDL_Window *displ_init_Window();
SDL_Renderer *displ_init_Renderer(SDL_Window *window);
SDL_Texture *displ_init_Texture(SDL_Renderer *renderer);

void displ_clear(chip8_t *chip8);
void displ_present(chip8_t *chip8);
void displ_destroy(chip8_t *chip8);

#endif
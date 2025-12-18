#include "SDL2/SDL.h"

#include "../include/display.h"
#include "../include/chip8.h"

#define WINDOW_WIDTH 	DISPLAY_WIDTH * 10 		// 64 * 10
#define WINDOW_HEIGHT 	DISPLAY_HEIGHT * 10 	// 32 * 10

int displ_init_SDL() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		fprintf(stderr, "Error initializing SDL! %s\n", SDL_GetError());
		return 1;
	}
	return 0;
}
SDL_Window *displ_init_Window() {
	SDL_Window *window = SDL_CreateWindow(
	 	"Chip 8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	
	if (window == NULL) {
		fprintf(stderr, "Error creating window! %s\n", SDL_GetError());
	}
	return window;
}

SDL_Renderer *displ_init_Renderer(SDL_Window *window) {
	if (window == NULL) return NULL;
	SDL_Renderer *renderer = SDL_CreateRenderer(window, 0, 0);

	if (renderer == NULL) {
		fprintf(stderr, "Error creating SDL renderer! %s\n", SDL_GetError());
	}
	return renderer;
}

SDL_Texture *displ_init_Texture(SDL_Renderer *renderer) {
	if (renderer == NULL) return NULL;
	// TODO High-res mode, need to remove the macro and do runtime checks
	SDL_Texture *texture = SDL_CreateTexture(renderer, 
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		DISPLAY_WIDTH,
		DISPLAY_HEIGHT);
	
	if (texture == NULL) {
		fprintf(stderr, "Error creating SDL texture! %s\n", SDL_GetError());
	}
	return texture;
}

void displ_clear(chip8_t *chip8) {
	// clear all screen bits
	for (int x = 0; x < DISPLAY_WIDTH; x++) {
		for (int y = 0; y < DISPLAY_HEIGHT; y++) {
			chip8->screen[x][y] = 0;
		}
	}
	// set to black
	SDL_SetRenderDrawColor(chip8->renderer, 0, 0, 0, 255);
	SDL_RenderClear(chip8->renderer);
	SDL_RenderPresent(chip8->renderer);
}

void displ_present(chip8_t *chip8) {
	uint32_t screen_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT] = {0};
	uint32_t pxl;

	for (uint8_t x = 0; x < DISPLAY_WIDTH; x++) {
		for (uint8_t y = 0; y < DISPLAY_HEIGHT; y++)
		{
			pxl = chip8->screen[x][y] * 0xFFFFFF00;
			// set alpha to 0xFF regardless if it's 0 or 1
			pxl |= 0x000000FF;
       	 	screen_buffer[y * DISPLAY_WIDTH + x] = pxl;
		}
	}
	SDL_UpdateTexture(chip8->texture, NULL, screen_buffer, DISPLAY_WIDTH * sizeof(uint32_t));
	SDL_RenderClear(chip8->renderer);
    SDL_RenderCopy(chip8->renderer, chip8->texture, NULL, NULL );
	SDL_RenderPresent(chip8->renderer);
}

void displ_destroy(chip8_t *chip8) {
	if (chip8->texture != NULL) SDL_DestroyTexture(chip8->texture);
	if (chip8->renderer != NULL) SDL_DestroyRenderer(chip8->renderer);
	if (chip8->window != NULL) SDL_DestroyWindow(chip8->window);
	SDL_Quit();
}

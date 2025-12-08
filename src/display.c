#include "../include/display.h"
#include "SDL2/SDL.h"

#define WINDOW_WIDTH DISPLAY_WIDTH * 10 	// 64 * 10
#define WINDOW_HEIGHT DISPLAY_HEIGHT * 10 	// 32 * 10

display_t displ_init(void) {
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		fprintf(stderr, "Error initializing SDL! %s\n", SDL_GetError());
		return NULL;
	}
	// zero out the screen array
	display_t displ = calloc(1, sizeof(struct display));
	if (displ == NULL) {
		fprintf(stderr, "Failed to allocate memory for display!\n");
		return NULL;
	}

	window = SDL_CreateWindow(
		"Chip 8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	if (window == NULL) {
		free(displ);
		fprintf(stderr, "Error creating window! %s\n", SDL_GetError());
		return NULL;
	}

	renderer = SDL_CreateRenderer(window, 0, 0);
	if (renderer == NULL) {
		SDL_DestroyWindow(window);
		free(displ);
		fprintf(stderr, "Error creating SDL renderer! %s\n", SDL_GetError());
		return NULL;
	}

	// apply scale 10 less, so 1 pixel equals 10
	if (SDL_RenderSetScale(renderer, 10.0f, 10.0f) < 0) {
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		free(displ);

		fprintf(stderr, "Error setting window scale! %s\n", SDL_GetError());
		return NULL;
	}

	displ->window = window;
	displ->renderer = renderer;
	
	// set to black
	SDL_SetRenderDrawColor(displ->renderer, 0, 0, 0, 255);
	SDL_RenderClear(displ->renderer);
	SDL_RenderPresent(displ->renderer);

	return displ;
}

void displ_destroy(display_t displ) {
	SDL_DestroyWindow(displ->window);
	SDL_DestroyRenderer(displ->renderer);
	SDL_Quit();

	free(displ);
}

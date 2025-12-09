#include "../include/input.h"
#include "SDL2/SDL.h"

void handle_input(chip8_t *chip8) {
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch(e.type) {
		case SDL_QUIT:
			chip8->running = 0;
			return;

		case SDL_KEYUP:
			// clear out the key
			chip8->key = 0;
			return;

		case SDL_KEYDOWN:

			switch (e.key.keysym.sym) {
			case SDLK_ESCAPE:
				chip8->running = 0;
				return;
			case SDLK_SPACE:
				// TODO maybe draw a pause on screen
				if (!chip8->paused) printf("PAUSE\n");
				chip8->paused = !chip8->paused;
				return;

			case SDLK_1:
				printf("Pressed 1\n");
				if (!chip8->paused) chip8->key = K_1;
				break;
			case SDLK_2:
				printf("Pressed 2\n");
				if (!chip8->paused) chip8->key = K_2;
				break;
			case SDLK_3:
				printf("Pressed 3\n");
				if (!chip8->paused) chip8->key = K_3;
				break;
			case SDLK_4:
				printf("Pressed 4\n");
				if (!chip8->paused) chip8->key = K_4;
				break;
			case SDLK_q:
				printf("Pressed Q\n");
				if (!chip8->paused) chip8->key = K_Q;
				break;
			case SDLK_w:
				printf("Pressed W\n");
				if (!chip8->paused) chip8->key = K_W;
				break;
			case SDLK_e:
				printf("Pressed E\n");
				if (!chip8->paused) chip8->key = K_E;
				break;
			case SDLK_r:
				printf("Pressed R\n");
				if (!chip8->paused) chip8->key = K_R;
				break;
			case SDLK_a:
				printf("Pressed A\n");
				if (!chip8->paused) chip8->key = K_A;
				break;
			case SDLK_s:
				printf("Pressed S\n");
				if (!chip8->paused) chip8->key = K_S;
				break;
			case SDLK_d:
				printf("Pressed D\n");
				if (!chip8->paused) chip8->key = K_D;
				break;
			case SDLK_f:
				printf("Pressed F\n");
				if (!chip8->paused) chip8->key = K_F;
				break;	
			case SDLK_z:
				printf("Pressed Z\n");
				if (!chip8->paused) chip8->key = K_Z;
				break;
			case SDLK_x:
				printf("Pressed X\n");
				if (!chip8->paused) chip8->key = K_X;
				break;
			case SDLK_c:
				printf("Pressed C\n");
				if (!chip8->paused) chip8->key = K_C;
				break;
			case SDLK_v:
				printf("Pressed V\n");
				if (!chip8->paused) chip8->key = K_V;
				break;
			default:
				break;
			}

		default:
			break;
		}
	}
}
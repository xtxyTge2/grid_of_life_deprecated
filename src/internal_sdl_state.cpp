#pragma once
#include <SDL.h>
#include <iostream>

class InternalSDLState {
public:
	InternalSDLState(float width, float height) {
		if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
			std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
			system("pause");
		}

		window = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED, width, height, NULL);
		if (!window) {
			std::cout << "Error creating window: " << SDL_GetError() << std::endl;
			system("pause");
		}

		renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_ACCELERATED);
		if (!renderer) {
			std::cout << "Error creating renderer: " << SDL_GetError() << std::endl;
		}

		int success = SDL_RenderClear(renderer);
		if (success != 0) {
			std::cout << "Error clearing the renderer: " << SDL_GetError() << std::endl;
		}
	}

	~InternalSDLState() {
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	SDL_Window* window;
	SDL_Renderer* renderer;
};

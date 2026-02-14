#pragma once

#include <SDL3/SDL.h>

struct SDLState
{
	SDL_Window* win;
	SDL_Renderer* renderer;
	int width;
	int height;
	int logH;
	int logW;
	const bool* keys;

	SDLState() : keys(SDL_GetKeyboardState(NULL))
	{

	}
};
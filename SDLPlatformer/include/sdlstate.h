#pragma once

#include <SDL3/SDL.h>

/**
 * @brief Holds the core SDL handles and window/presentation state shared
 * throughout the application.
 */
struct SDLState
{
	SDL_Window* win;			// The application's main window
	SDL_Renderer* renderer;	// Renderer used for all drawing
	int width;					// Current window width, in pixels (updated on resize)
	int height;					// Current window height, in pixels (updated on resize)
	int logH;					// Logical (render-resolution) height used for letterboxed presentation
	int logW;					// Logical (render-resolution) width used for letterboxed presentation
	const bool* keys;			// Live keyboard state array indexed by SDL_Scancode

	SDLState() : keys(SDL_GetKeyboardState(NULL))
	{

	}
};
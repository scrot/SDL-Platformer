// SDLPlatformer.cpp : Defines the entry point for the application.
//
#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>
#include<SDL3_image/SDL_image.h>

#include "SDLPlatformer.h"

using namespace std;

struct SDL_State
{
	SDL_Window *win;
	SDL_Renderer *renderer;
};

void cleanup(SDL_State &state);

int main(int argc, char *argv[])
{
	int width = 800;
	int height = 600;
	bool isRunning = true;
	SDL_State state;

	// Initialize the SDL video subsystem
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Unable to initialize SDL", NULL);

		return -1;
	}

	state.win = SDL_CreateWindow("SDL3 Demo", width, height, 0);

	if (!state.win)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Unable to create SDL Window", state.win);
		cleanup(state);

		return -1;
	}

	// Create the renderer
	state.renderer = SDL_CreateRenderer(state.win, NULL);

	if (!state.renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Unable to create SDL Renderer", state.win);
		cleanup(state);
		return -2;
	}

	// Load game assets here
	SDL_Texture* idleTexture = IMG_LoadTexture(state.renderer, "assets/idle.png");

	// Start the main game loop
	while (isRunning)
	{
		// Check events first
		SDL_Event event{ 0 };

		while(SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_EVENT_QUIT:
				{
					isRunning = false;
					break;
				}
				default:
				{
					break;
				}
			}
		}

		// Perform drawing commands

		// Set the background color and clear the back buffer
		SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
		SDL_RenderClear(state.renderer);

		// Render the idle texture
		SDL_RenderTexture(state.renderer, idleTexture, NULL, NULL);

		// Swap buffers and present the back buffer
		SDL_RenderPresent(state.renderer);
	}
	cout << "Hello CMake." << endl;

	// Free game assets here
	SDL_DestroyTexture(idleTexture);
	cleanup(state);

	return 0;
}

void cleanup(SDL_State &state)
{
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.win);

	SDL_Quit();
}

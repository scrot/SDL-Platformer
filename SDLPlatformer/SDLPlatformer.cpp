// SDLPlatformer.cpp : Defines the entry point for the application.
//
#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>

#include "SDLPlatformer.h"

using namespace std;

struct SDL_State
{
	SDL_Window *win;
	SDL_Renderer *renderer;
};

void cleanup(SDL_Window* win);

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
		cleanup(state.win);

		return -1;
	}

	// Start the main game loop
	while (isRunning)
	{
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
	}
	cout << "Hello CMake." << endl;

	cleanup(state.win);

	return 0;
}

void cleanup(SDL_State &state)
{
	SDL_DestroyWindow(state.win);

	SDL_Quit();
}

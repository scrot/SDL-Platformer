// SDLPlatformer.cpp : Defines the entry point for the application.
//
#include<SDL3/SDL.h>

#include "SDLPlatformer.h"

using namespace std;

int main()
{
	// Initialize the SDL video subsystem
	SDL_Init(SDL_INIT_VIDEO);

	cout << "Hello CMake." << endl;

	SDL_Quit();

	return 0;
}

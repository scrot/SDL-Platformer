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
	int width;
	int height;
	int logH;
	int logW;
};

// Function prototypes
void cleanup(SDL_State &state);
bool initialize(SDL_State& state);

int main(int argc, char *argv[])
{
	uint64_t prevTime = SDL_GetTicks();

	bool isRunning = true;
	bool flipHorizontal = false;

	SDL_State state;
	state.width = 1600;
	state.height = 900;
	state.logW = 640;
	state.logH = 320;

	const float spriteSize = 32;

	// Initialize window and renderer
	if (!initialize(state))
	{
		return -1;
	}

	// Load game assets here

	// Load the idle texture
	SDL_Texture* idleTexture = IMG_LoadTexture(state.renderer, "assets/idle.png");

	// Set the texture scale mode to nearest neighbor (pixelated)
	SDL_SetTextureScaleMode(idleTexture, SDL_ScaleMode::SDL_SCALEMODE_NEAREST);

	// Setup game data here
	const bool *keys = SDL_GetKeyboardState(NULL);
	float playerX = 150;
	const float floor = state.logH;

	// Start the main game loop
	while (isRunning)
	{
		uint64_t nowTime = SDL_GetTicks();
		float deltaTime = (nowTime - prevTime) / 1000.0f; // Convert from milliseconds to seconds
		prevTime = nowTime;	// Update the game state here using deltaTime

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
				case SDL_EVENT_WINDOW_RESIZED:
				{
					state.width = event.window.data1;
					state.height = event.window.data2;
					break;
				}
				default:
				{
					break;
				}
			}
		}

		// Handle movement input
		float moveAmount = 0.0f;
		if (keys[SDL_SCANCODE_A])
		{
			moveAmount = -75.0f;
			flipHorizontal = true;
		}	
		else
			if (keys[SDL_SCANCODE_D])
			{
				moveAmount += 75.0f;
				flipHorizontal = false;
			}
				

		playerX += moveAmount * deltaTime;

		// Perform drawing commands

		// Set the background color and clear the back buffer
		SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
		SDL_RenderClear(state.renderer);

		// Render the idle texture
		SDL_FRect src
		{
			.x = 0,
			.y = 0,
			.w = spriteSize,
			.h = spriteSize
		};

		SDL_FRect dst
		{
			.x = playerX,
			.y = floor - spriteSize,
			.w = spriteSize,
			.h = spriteSize
		};

		SDL_RenderTextureRotated(state.renderer, idleTexture, &src, &dst, 0, nullptr, 
			(flipHorizontal == true)? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

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

bool initialize(SDL_State& state)
{
	bool initSuccess = true;

	// Initialize the SDL video subsystem
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Unable to initialize SDL", NULL);

		initSuccess = false;
	}

	state.win = SDL_CreateWindow("SDL3 Demo", state.width, state.height, SDL_WINDOW_RESIZABLE);

	if (!state.win)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Unable to create SDL Window", state.win);
		cleanup(state);

		initSuccess = false;
	}

	// Create the renderer
	state.renderer = SDL_CreateRenderer(state.win, NULL);

	if (!state.renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Unable to create SDL Renderer", state.win);
		cleanup(state);
		initSuccess = false;
	}

	// Configure presentation
	SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	return initSuccess;

}

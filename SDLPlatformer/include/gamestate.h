#pragma once

#include "sdlstate.h"

// Structure to hold all gane info, including game objects, player index, etc.
struct GameState
{
	std::array<std::vector<GameObject>, 2> layers;
	std::vector<GameObject> backgroundTiles;
	std::vector<GameObject> foregroundTiles;
	std::vector<GameObject> bullets;

	int playerIndex;
	int playerLayer;
	SDL_FRect mapViewport;
	float bg2Scroll;
	float bg3Scroll;
	float bg4Scroll;
	bool debugMode;

	GameState(const SDLState& state)
	{
		playerIndex = -1;
		mapViewport = SDL_FRect
		{
			.x = 0,
			.y = 0,
			.w = static_cast<float>(state.logW),
			.h = static_cast<float>(state.logH)
		};

		bg2Scroll = 0;
		bg3Scroll = 0;
		bg4Scroll = 0;

		debugMode = false;
	}

	GameObject& player()
	{
		return layers[playerLayer][playerIndex];
	}
};
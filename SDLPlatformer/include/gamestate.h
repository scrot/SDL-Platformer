#pragma once

#include "sdlstate.h"

/**
 * @brief Holds all per-level game data: live objects, the camera, and
 * parallax/debug state. Constructed once per level and mutated in place
 * every frame by the game loop in SDLPlatformer.cpp.
 */
struct GameState
{
	// Objects that participate in per-frame update()/collision, indexed by
	// LAYER_IDX_LEVEL (static geometry) and LAYER_IDX_CHARACTERS (player/enemies)
	std::array<std::vector<GameObject>, 2> layers;
	std::vector<GameObject> backgroundTiles;	// Purely visual tiles drawn behind the object layers
	std::vector<GameObject> foregroundTiles;	// Purely visual tiles drawn in front of the object layers
	std::vector<GameObject> bullets;			// Pool of bullet objects; inactive ones are recycled rather than removed

	int playerIndex;		// Index of the player within layers[LAYER_IDX_CHARACTERS]
	SDL_FRect mapViewport;	// Current camera rectangle in world space, used to offset rendering
	float bg2Scroll;		// Current horizontal scroll offset for parallax background layer 2
	float bg3Scroll;		// Current horizontal scroll offset for parallax background layer 3
	float bg4Scroll;		// Current horizontal scroll offset for parallax background layer 4
	bool debugMode;			// Whether the on-screen debug overlay is enabled (toggled with F5)

	/**
	 * @brief Initializes the map viewport to the logical screen size and
	 * resets scroll/debug state. No game objects exist yet after
	 * construction -- callers (e.g. createTiles()) are responsible for
	 * populating layers/tiles/bullets and setting playerIndex.
	 *
	 * @param state -- SDLState object used to size the initial viewport.
	 */
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

	/**
	 * @brief Convenience accessor for the player's GameObject.
	 *
	 * @return Reference to the player object in layers[LAYER_IDX_CHARACTERS].
	 */
	GameObject& player()
	{
		return layers[LAYER_IDX_CHARACTERS][playerIndex];
	}
};
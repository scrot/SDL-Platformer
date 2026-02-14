#pragma once

#include <vector>

#include <SDL3_image/SDL_image.h>

#include "animation.h"
#include "sdlstate.h"

struct Resources
{
	const int ANIM_PLAYER_IDLE = 0; // Index in the animation vector for the player idle animation
	const int ANIM_PLAYER_RUNNING = 1;	// Index for running animation
	const int ANIM_PLAYER_SLIDE = 2; // Index for sliding animation

	std::vector<Animation> playerAnims;

	std::vector<SDL_Texture*> textures;
	SDL_Texture* texIdle;
	SDL_Texture* texRunning;
	SDL_Texture* texBrick;
	SDL_Texture* texGrass;
	SDL_Texture* texGround;
	SDL_Texture* texPanel;
	SDL_Texture* texSlide;
	SDL_Texture* texBg1;
	SDL_Texture* texBg2;
	SDL_Texture* texBg3;
	SDL_Texture* texBg4;

	/* @brief Load the requested texture.
	*
	*	@param renderer -- SDL_Renderer object
	*	@param filePath -- The path to the requested texture.
	*
	*	@return The loaded SDL_Texture object.
	*/
	SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& filePath)
	{
		SDL_Texture* tex = IMG_LoadTexture(renderer, filePath.c_str());	// Convert the file path to a C-style string

		// Set the texture scale mode to nearest neighbor (pixelated)
		SDL_SetTextureScaleMode(tex, SDL_ScaleMode::SDL_SCALEMODE_NEAREST);

		textures.push_back(tex);

		return tex;
	}

	/**
	 * @brief Load all required textures.
	 *
	 * @param state -- SDLState object that contains the renderer info.
	 */
	void load(SDLState& state)
	{
		playerAnims.resize(5);	// There are 5 player animations, so we resize the vector to accomodate them.
		playerAnims[ANIM_PLAYER_IDLE] = Animation(8, 1.6f);	// There are 8 frames in the idle animation that lasts 1.6 seconds.
		playerAnims[ANIM_PLAYER_RUNNING] = Animation(4, 0.5f); // 4 frames in running animation that lasts 0.5 seconds.
		playerAnims[ANIM_PLAYER_SLIDE] = Animation(1, 1.0f); // 1 frame in sliding animation that lasts 1 second

		// Load the animations
		texIdle = loadTexture(state.renderer, "assets/idle.png");
		texRunning = loadTexture(state.renderer, "assets/run.png");
		texBrick = loadTexture(state.renderer, "assets/tiles/brick.png");
		texGrass = loadTexture(state.renderer, "assets/tiles/grass.png");
		texGround = loadTexture(state.renderer, "assets/tiles/ground.png");
		texPanel = loadTexture(state.renderer, "assets/tiles/panel.png");
		texSlide = loadTexture(state.renderer, "assets/slide.png");
		texBg1 = loadTexture(state.renderer, "assets/bg/bg_layer1.png");
		texBg2 = loadTexture(state.renderer, "assets/bg/bg_layer2.png");
		texBg3 = loadTexture(state.renderer, "assets/bg/bg_layer3.png");
		texBg4 = loadTexture(state.renderer, "assets/bg/bg_layer4.png");
	}

	/**
	 * @brief Free all loaded textures.
	 */
	void unload()
	{
		for (SDL_Texture* tex : textures)
		{
			SDL_DestroyTexture(tex);
		}
	}
};
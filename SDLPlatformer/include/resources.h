#pragma once

#include <iostream>
#include <vector>

#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "animation.h"
#include "sdlstate.h"
#include "tmx.h"

using namespace std;

struct Resources
{
	struct TileSetTextures
	{
		int firstGid;
		std::vector<SDL_Texture*> textures;
	};

	const int ANIM_PLAYER_IDLE = 0; // Index in the animation vector for the player idle animation
	const int ANIM_PLAYER_RUNNING = 1;	// Index for running animation
	const int ANIM_PLAYER_SLIDE = 2; // Index for sliding animation
	const int ANIM_PLAYER_SHOOT = 3; // Index for player shooting animation
	const int ANIM_PLAYER_SLIDESHOOT = 4;	// Index for player sliding/shooting animation
	const int ANIM_BULLET_MOVING = 0; // Index for bullet moving animation
	const int ANIM_BULLET_HIT = 1; // Index for bullet hit animation
	const int ANIM_ENEMY = 0;
	const int ANIM_ENEMY_HIT = 1;
	const int ANIM_ENEMY_DIE = 2;

	std::vector<Animation> playerAnims;
	std::vector<Animation> bulletAnims;
	std::vector<Animation> enemyAnims;

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
	SDL_Texture* texBullet;
	SDL_Texture* texBulletHit;
	SDL_Texture* texShoot;
	SDL_Texture* texRunShoot;
	SDL_Texture* texSlideShoot;
	SDL_Texture* texEnemy;
	SDL_Texture* texEnemyHit;
	SDL_Texture* texEnemyDie;

	MIX_Audio* chunkShoot;
	MIX_Audio* chunkShootHit;
	MIX_Audio* chunkEnemyHit;
	MIX_Audio* chunkEnemyDie;
	MIX_Audio* levelMusic;

	std::vector<MIX_Audio*> chunks;
	MIX_Mixer* mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);

	std::vector<TileSetTextures> tilesetTextures;
	std::unique_ptr<tmx::Map> map;

	MIX_Audio* loadChunk(const std::string &filepath)
	{
		MIX_Audio* chunk = MIX_LoadAudio(mixer, filepath.c_str(), true);

		chunks.push_back(chunk);

		return chunk;
	}

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
		playerAnims[ANIM_PLAYER_SHOOT] = Animation(4, 0.5f); // 4 frames in shooting that lasts 0.5 seconds
		playerAnims[ANIM_PLAYER_SLIDESHOOT] = Animation(4, 0.5f); // 4 frames in sliding/shooting animation that lasts 0.5 seconds

		bulletAnims.resize(2);
		bulletAnims[ANIM_BULLET_MOVING] = Animation(4, 0.05f);
		bulletAnims[ANIM_BULLET_HIT] = Animation(4, 0.15f);

		enemyAnims.resize(3);
		enemyAnims[ANIM_ENEMY] = Animation(8, 1.0f);
		enemyAnims[ANIM_ENEMY_HIT] = Animation(8, 1.0f);
		enemyAnims[ANIM_ENEMY_DIE] = Animation(18, 2.0f);

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
		texBullet = loadTexture(state.renderer, "assets/bullet.png");
		texBulletHit = loadTexture(state.renderer, "assets/bullethit.png");
		texShoot = loadTexture(state.renderer, "assets/shoot.png");
		texRunShoot = loadTexture(state.renderer, "assets/shoot_run.png");
		texSlideShoot = loadTexture(state.renderer, "assets/slide_shoot.png");
		texEnemy = loadTexture(state.renderer, "assets/enemy.png");
		texEnemyHit = loadTexture(state.renderer, "assets/enemy_hit.png");
		texEnemyDie = loadTexture(state.renderer, "assets/enemy_die.png");

		// Load audio sounds
		chunkShoot = loadChunk("assets/audio/shoot.wav");
		chunkShootHit = loadChunk("assets/audio/wall_hit.wav");
		chunkEnemyHit = loadChunk("assets/audio/enemy_hit.wav");
		chunkEnemyDie = loadChunk("assets/audio/enemy_die.wav");
		levelMusic = loadChunk("assets/audio/levelMusic.mp3");

		// Load maps
		map = tmx::loadMap("assets/map/smallmap.tmx");

		for (tmx::TileSet& tileSet : map->tileSets)
		{
			TileSetTextures tst;
			tst.firstGid = tileSet.firstgid;
			tst.textures.reserve(tileSet.tiles.size());

			for (tmx::Tile& tile : tileSet.tiles)
			{
				const std::string imagePath = "assets/tiles/" + std::filesystem::path(tile.image.source).filename().string();
				tst.textures.push_back(loadTexture(state.renderer, imagePath));
			}

			tilesetTextures.push_back(std::move(tst));
		}
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
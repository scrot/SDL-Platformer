// SDLPlatformer.cpp : Defines the entry point for the application.
//
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <string>
#include <array>

#include "include/SDLPlatformer.h"
#include "include/gameobject.h"

using namespace std;

// Global constants
const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_CHARACTERS = 1;

struct SDL_State
{
	SDL_Window *win;
	SDL_Renderer *renderer;
	int width;
	int height;
	int logH;
	int logW;
	const bool* keys;

	SDL_State() : keys(SDL_GetKeyboardState(NULL))
	{

	}
};

struct Resources
{
	const int ANIM_PLAYER_IDLE = 0; // Index in the animation vector for the player idle animation
	std::vector<Animation> playerAnims;

	std::vector<SDL_Texture*> textures;
	SDL_Texture* texIdle;

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
	 * @param state -- SDL_State object that contains the renderer info.
	 */
	void load(SDL_State& state)
	{
		playerAnims.resize(5);	// There are 5 player animations, so we resize the vector to accomodate them.
		playerAnims[ANIM_PLAYER_IDLE] = Animation(8, 1.6f);	// There are 8 frames in the idle animation that lasts 1.6 seconds.

		// Load the idle texture
		texIdle = loadTexture(state.renderer, "assets/idle.png");
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

// Streucture to hold all gane info, including game objects, player index, etc.
struct GameState
{
	std::array<std::vector<GameObject>, 2> layers;
	int playerIndex;

	GameState()
	{
		playerIndex = 0;	// WILL CHANGE THIS WHEN WE LOAD MAPS
	}
};

// Function prototypes
void cleanup(SDL_State &state);
bool initialize(SDL_State& state);
void drawObject(const SDL_State& state, GameState& gs, GameObject& obj, float deltaTime);
void update(const SDL_State& state, GameState& gs, Resources& rs, GameObject& obj, float deltaTime);

int main(int argc, char *argv[])
{
	uint64_t prevTime = SDL_GetTicks();

	bool isRunning = true;

	SDL_State state;
	state.width = 1600;
	state.height = 900;
	state.logW = 640;
	state.logH = 320;

	

	// Initialize window and renderer
	if (!initialize(state))
	{
		return -1;
	}

	// Load game assets
	Resources res;
	res.load(state);
	

	// Setup game data here
	GameState gs;
	GameObject player;

	// Create player object
	player.type = ObjectType::player;
	player.texture = res.texIdle;
	player.animations = res.playerAnims;
	player.currentAnimation = res.ANIM_PLAYER_IDLE;
	player.acceleration = glm::vec2(300, 0);
	player.maxSpeedX = 100;

	gs.layers[LAYER_IDX_CHARACTERS].push_back(player);

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

		// Perform drawing commands

		// Update all objects
		for (auto& layer : gs.layers)
		{
			for (GameObject obj : layer)
			{
				update(state, gs, res, obj, deltaTime);

				if (obj.currentAnimation != -1)
				{
					obj.animations[obj.currentAnimation].step(deltaTime);
				}
			}
		}

		// Set the background color and clear the back buffer
		SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
		SDL_RenderClear(state.renderer);

		// Draw all objects
		for (auto &layer : gs.layers)
		{
			for (GameObject &obj : layer)
			{
				drawObject(state, gs, obj, deltaTime);
			}
		}
		
		// Swap buffers and present the back buffer
		SDL_RenderPresent(state.renderer);
	}

	cout << "Hello CMake." << endl;

	// Free game assets here
	res.unload();
	cleanup(state);

	return 0;
}

/* 
* @brief Clean up and free SDL resources
* 
* @param state -- state object that contains the window and renderer info.
*/
void cleanup(SDL_State &state)
{
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.win);

	SDL_Quit();
}

/*
* @brief Initialize SDL libraries and set up the render window
* @param state -- state object that contains the window and renderer info.
* 
* @return true if initialization was successful, false otherwise.
*/
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

void drawObject(const SDL_State &state, GameState &gs, GameObject &obj, float deltaTime)
{ 
	const float spriteSize = 32;
	float srcX = obj.currentAnimation != -1 ? obj.animations[obj.currentAnimation].currentFrame() * spriteSize : 0;

	// The source rectangle (animation frame) that we want o render from the sprite sheet
	SDL_FRect src
	{
		.x = srcX,
		.y = 0,
		.w = spriteSize,
		.h = spriteSize
	};

	// The destination rectangle (position on the screen) that we want to render to
	SDL_FRect dst
	{
		.x = obj.position.x,
		.y = obj.position.y,
		.w = spriteSize,
		.h = spriteSize
	};

	SDL_FlipMode flipMode = obj.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

	SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
}

void update(const SDL_State& state, GameState& gs, Resources& rs, GameObject &obj, float deltaTime)
{
	if (obj.type == ObjectType::player)
	{
		float currentDirection = 0.0f;

		if (state.keys[SDL_SCANCODE_A])
			currentDirection += -1.0f;

		if (state.keys[SDL_SCANCODE_D])
			currentDirection += 1.0f;

		if (currentDirection)
			obj.direction = currentDirection;

		switch (obj.data.playerData.state)
		{
			case PlayerState::idle:
			{
				if (currentDirection)
					obj.data.playerData.state = PlayerState::running;
				break;
			}
			case PlayerState::jumping:
			{
				break;
			}
			case PlayerState::running:
			{
				if (!currentDirection)
					obj.data.playerData.state = PlayerState::idle;
				break;
			}
			case PlayerState::crouching:
			{
				break;
			}
			default:
			{
				break;
			}
		}

		// Add acceleration to velocity
		obj.velocity += currentDirection * obj.acceleration * deltaTime;

		// Add velocity to position
		obj.position += obj.velocity * deltaTime;
	}
}

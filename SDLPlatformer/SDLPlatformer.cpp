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
const int MAP_ROWS = 5;
const int MAP_COLS = 50;
const int TILE_SIZE = 32;

struct SDLState
{
	SDL_Window *win;
	SDL_Renderer *renderer;
	int width;
	int height;
	int logH;
	int logW;
	const bool* keys;

	SDLState() : keys(SDL_GetKeyboardState(NULL))
	{

	}
};

struct Resources
{
	const int ANIM_PLAYER_IDLE = 0; // Index in the animation vector for the player idle animation
	const int ANIM_PLAYER_RUNNING = 1;	// Index for running animation

	std::vector<Animation> playerAnims;

	std::vector<SDL_Texture*> textures;
	SDL_Texture* texIdle;
	SDL_Texture* texRunning;
	SDL_Texture* texBrick;
	SDL_Texture* texGrass;
	SDL_Texture* texGround;
	SDL_Texture* texPanel;

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

		// Load the animations
		texIdle = loadTexture(state.renderer, "assets/idle.png");
		texRunning = loadTexture(state.renderer, "assets/run.png");
		texBrick = loadTexture(state.renderer, "assets/tiles/brick.png");
		texGrass = loadTexture(state.renderer, "assets/tiles/grass.png");
		texGround = loadTexture(state.renderer, "assets/tiles/ground.png");
		texPanel = loadTexture(state.renderer, "assets/tiles/panel.png");
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
void cleanup(SDLState &state);
bool initialize(SDLState& state);
void drawObject(const SDLState& state, GameState& gs, GameObject& obj, float deltaTime);
void update(const SDLState& state, GameState& gs, Resources& rs, GameObject& obj, float deltaTime);
void createTiles(const SDLState& state, GameState& gs, const Resources& res);

int main(int argc, char *argv[])
{
	uint64_t prevTime = SDL_GetTicks();

	bool isRunning = true;

	SDLState state;
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

	createTiles(state, gs, res);

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
			for (GameObject &obj : layer)
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
void cleanup(SDLState &state)
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
bool initialize(SDLState& state)
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

void drawObject(const SDLState &state, GameState &gs, GameObject &obj, float deltaTime)
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

void update(const SDLState& state, GameState& gs, Resources& rs, GameObject &obj, float deltaTime)
{
	if (obj.dynamic)
	{
		// Add gravity
		obj.velocity += glm::vec2(0, 500) * deltaTime;
	}
	

	if (obj.type == ObjectType::player)
	{
		float currentDirection = 0.0f;

		if (state.keys[SDL_SCANCODE_A] || state.keys[SDL_SCANCODE_LEFT])
			currentDirection += -1.0f;

		if (state.keys[SDL_SCANCODE_D] || state.keys[SDL_SCANCODE_RIGHT])
			currentDirection += 1.0f;

		if (currentDirection)
			obj.direction = currentDirection;

		switch (obj.data.playerData.state)
		{
			case PlayerState::idle:
			{
				if (currentDirection)
				{
					obj.data.playerData.state = PlayerState::running;
					obj.texture = rs.texRunning;
					obj.currentAnimation = rs.ANIM_PLAYER_RUNNING;
				}
				else
				{
					// Slow down the player when idle
					const float factor = (obj.velocity.x > 0) ? -1.5f : 1.5f;
					float amount = factor * obj.acceleration.x * deltaTime;

					if (std::abs(obj.velocity.x) < std::abs(amount))
						obj.velocity.x = 0;
					else
						obj.velocity.x += amount;
				}
				break;
			}
			case PlayerState::jumping:
			{
				break;
			}
			case PlayerState::running:
			{
				if (!currentDirection)
				{
					obj.data.playerData.state = PlayerState::idle;
					obj.texture = rs.texIdle;
					obj.currentAnimation = rs.ANIM_PLAYER_IDLE;
				}
				break;
			}
			case PlayerState::crouching:
			{
				break;
			}
			default:
				break;
		}

		// Add acceleration to velocity
		obj.velocity += currentDirection * obj.acceleration * deltaTime;

		// Clamp velocity to max speed
		if (std::abs(obj.velocity.x) > obj.maxSpeedX)
		{
			obj.velocity.x = obj.direction * obj.maxSpeedX;
		}

		
	}

	// Add velocity to position
	obj.position += obj.velocity * deltaTime;
}

void createTiles(const SDLState& state, GameState& gs, const Resources& res)
{
	/*
		1 -- Ground
		2 -- Panel
		3 -- Enemy
		4 -- Player
		5 -- Grass
		6 -- Brick
	*/
	short map[MAP_ROWS][MAP_COLS] =
	{
		4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	const auto createObject = [&state](int row, int col, SDL_Texture* tex, ObjectType type)
	{
		GameObject obj;
		obj.type = type;
		obj.texture = tex;
		obj.position = glm::vec2(col * TILE_SIZE, state.logH - (MAP_ROWS - row) * TILE_SIZE);

		return obj;
	};

	for (int row = 0; row < MAP_ROWS; row++)
		for (int col = 0; col < MAP_COLS; col++)
		{
			switch (map[row][col])
			{
				case 1: // Ground
				{
					GameObject ground = createObject(row, col, res.texGround, ObjectType::level);
					gs.layers[LAYER_IDX_LEVEL].push_back(ground);
					break;
				}
				case 2: // Panel
				{
					GameObject ground = createObject(row, col, res.texPanel, ObjectType::level);
					gs.layers[LAYER_IDX_LEVEL].push_back(ground);
					break;
				}
				case 4: // Player
				{
					// Create player object
					GameObject player = createObject(row, col, res.texIdle, ObjectType::player);

					player.position = glm::vec2(col * TILE_SIZE, 
												state.logH - (MAP_ROWS - row) * TILE_SIZE);
					player.animations = res.playerAnims;
					player.currentAnimation = res.ANIM_PLAYER_IDLE;
					player.acceleration = glm::vec2(300, 0);
					player.maxSpeedX = 100;
					player.data.playerData = PlayerData();
					player.dynamic = true;

					gs.layers[LAYER_IDX_CHARACTERS].push_back(player);

					break;
				}
			}
		}
}

// SDLPlatformer.cpp : Defines the entry point for the application.
//
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <string>
#include <array>
#include <format>

#include "include/SDLPlatformer.h"
#include "include/gameobject.h"
#include "include/gamestate.h"
#include "include/resources.h"
#include "include/sdlstate.h"

using namespace std;



// Function prototypes
void cleanup(SDLState &state);
bool initialize(SDLState& state);
void drawObject(const SDLState& state, GameState& gs, GameObject& obj, float width, float height, float deltaTime);
void update(const SDLState& state, GameState& gs, Resources& rs, GameObject& obj, float deltaTime);
void createTiles(const SDLState& state, GameState& gs, const Resources& res);
void collisionResponse(const SDLState& state, GameState& gs, Resources& res, const SDL_FRect& rectA, const SDL_FRect& rectB,
					   const SDL_FRect& intersection, GameObject& objA, GameObject& objB, float deltaTime);
void checkCollision(const SDLState& state, GameState& gs, Resources& res, GameObject& a, GameObject& b, float deltaTime);
void handleKeyInput(const SDLState& state, GameState& gs, GameObject& obj, SDL_Scancode key, bool keyDown);
void drawParallaxBackground(SDL_Renderer* renderer, SDL_Texture* texture, float xVelocity,
	float& scrollPos, float scrollFactor, float deltaTime);

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
	GameState gs(state);
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
				case SDL_EVENT_KEY_DOWN:
				{
					if (event.key.scancode == SDL_SCANCODE_ESCAPE)
						isRunning = false;
					else
						handleKeyInput(state, gs, gs.player(), event.key.scancode, true);

					break;
				}
				case SDL_EVENT_KEY_UP:
				{
					handleKeyInput(state, gs, gs.player(), event.key.scancode, false);

					if (event.key.scancode == SDL_SCANCODE_F5)
						gs.debugMode = !gs.debugMode;

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

		// Update bullets
		for (GameObject& bullet : gs.bullets)
		{
			update(state, gs, res, bullet, deltaTime);

			if (bullet.currentAnimation != -1)
			{
				bullet.animations[bullet.currentAnimation].step(deltaTime);
			}
		}

		// Calculate viewport position
		gs.mapViewport.x = (gs.player().position.x + TILE_SIZE / 2) - gs.mapViewport.w / 2;

		// Perform drawing commands
		SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
		SDL_RenderClear(state.renderer);

		// Draw background images
		SDL_RenderTexture(state.renderer, res.texBg1, nullptr, nullptr);
		drawParallaxBackground(state.renderer, res.texBg4, gs.player().velocity.x, gs.bg4Scroll, 0.075f, deltaTime);
		drawParallaxBackground(state.renderer, res.texBg3, gs.player().velocity.x, gs.bg3Scroll, 0.15f, deltaTime);
		drawParallaxBackground(state.renderer, res.texBg2, gs.player().velocity.x, gs.bg2Scroll, 0.3f, deltaTime);

		// Draw background tiles
		for (GameObject &obj : gs.backgroundTiles)
		{
			SDL_FRect dst =
			{
				.x = obj.position.x - gs.mapViewport.x,
				.y = obj.position.y,
				.w = static_cast<float>(obj.texture->w),
				.h = static_cast<float>(obj.texture->h)
			};

			SDL_RenderTexture(state.renderer, obj.texture, nullptr, &dst);
		}

		// Draw all objects
		for (auto &layer : gs.layers)
		{
			for (GameObject &obj : layer)
			{
				drawObject(state, gs, obj, TILE_SIZE, TILE_SIZE, deltaTime);
			}
		}

		// Draw bullets
		for (GameObject& bullet : gs.bullets)
		{
			drawObject(state, gs, bullet, bullet.collider.w, bullet.collider.h, deltaTime);
		}

		// Draw foreground tiles
		for (GameObject& obj : gs.foregroundTiles)
		{
			SDL_FRect dst =
			{
				.x = obj.position.x - gs.mapViewport.x,
				.y = obj.position.y,
				.w = static_cast<float>(obj.texture->w),
				.h = static_cast<float>(obj.texture->h)
			};

			SDL_RenderTexture(state.renderer, obj.texture, nullptr, &dst);
		}

		if (gs.debugMode)
		{
			// Display debug info	
			SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
			SDL_RenderDebugText(state.renderer, 5, 5,
					std::format("S: {}, B: {} G: {}",
					static_cast<int>(gs.player().data.playerData.state), gs.bullets.size(), gs.player().grounded).c_str());
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

	SDL_SetRenderVSync(state.renderer, 1);

	// Configure presentation
	SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	return initSuccess;

}

void drawObject(const SDLState &state, GameState &gs, GameObject &obj, float width, float height, float deltaTime)
{ 
	float srcX = obj.currentAnimation != -1 ? obj.animations[obj.currentAnimation].currentFrame() * width : 0;

	// The source rectangle (animation frame) that we want o render from the sprite sheet
	SDL_FRect src
	{
		.x = srcX,
		.y = 0,
		.w = width,
		.h = height
	};

	// The destination rectangle (position on the screen) that we want to render to
	SDL_FRect dst
	{
		.x = obj.position.x - gs.mapViewport.x,
		.y = obj.position.y,
		.w = width,
		.h = height
	};

	SDL_FlipMode flipMode = obj.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	cout << "Y VELOCITY: " << obj.velocity.y << endl;	// Command to force jumping to work. Still need to figure out the root cause of the problem

	SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);

	// Print out debug stuff
	if (gs.debugMode)
	{
		SDL_FRect rectA
		{
			.x = obj.position.x + obj.collider.x - gs.mapViewport.x,
			.y = obj.position.y + obj.collider.y,
			.w = obj.collider.w,
			.h = obj.collider.h
		};

		SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 150);
		SDL_RenderFillRect(state.renderer, &rectA);
		SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);
	}
}

void update(const SDLState& state, GameState& gs, Resources& rs, GameObject &obj, float deltaTime)
{
	float currentDirection = 0.0f;

	if (obj.dynamic && !obj.grounded)
	{
		// Add gravity
		obj.velocity += glm::vec2(0, 500) * deltaTime;
	}
	

	if (obj.type == ObjectType::player)
	{		
		if (state.keys[SDL_SCANCODE_A] || state.keys[SDL_SCANCODE_LEFT])
			currentDirection += -1.0f;

		if (state.keys[SDL_SCANCODE_D] || state.keys[SDL_SCANCODE_RIGHT])
			currentDirection += 1.0f;

		

		// Get a reference to weapon's timer without having to spell out
		// obj.data.playerData.weaponTimer each time
		Timer& weaponTimer = obj.data.playerData.weaponTimer;
		weaponTimer.step(deltaTime);

		const auto handleShooting = [&state, &gs, &rs, &obj, &weaponTimer]
									(SDL_Texture *tex, SDL_Texture *shootTex, int animIndex, int shootAnimIndex)
		{
			if (state.keys[SDL_SCANCODE_J])
			{
				// Set shooting texture/animation
				obj.texture = shootTex;
				obj.currentAnimation = shootAnimIndex;

				if (weaponTimer.isTimeout())
				{
					weaponTimer.reset();

					// Spawn bullets
					GameObject bullet;

					bullet.type = ObjectType::bullet;
					bullet.direction = gs.player().direction;
					bullet.texture = rs.texBullet;
					bullet.currentAnimation = rs.ANIM_BULLET_MOVING;
					bullet.collider = SDL_FRect
					{
						.x = 0,
						.y = 0,
						.w = static_cast<float>(rs.texBullet->w),
						.h = static_cast<float>(rs.texBullet->h)
					};
					bullet.velocity = glm::vec2(obj.velocity.x + 600.0f * obj.direction, 0);
					bullet.maxSpeedX = 1000.0f;
					bullet.animations = rs.bulletAnims;

					// Adjust bullet start position
					const float left = 4;
					const float right = 24;
					const float t = (obj.direction + 1) / 2.0f;
					const float xOffset = (left + right) * t; // LERP between left and right based on direction

					bullet.position = glm::vec2(
						obj.position.x + xOffset,
						obj.position.y + (TILE_SIZE / 2) + 1
					);

					// Check for inactive bullets and replace them
					bool foundInactive = false;

					for (int i = 0; i < gs.bullets.size() && !foundInactive; i++)
					{
						if (gs.bullets[i].data.bulletData.state == BulletState::inactive)
						{
							foundInactive = true;
							gs.bullets[i] = bullet;
						}
							
					}

					// If no inactive slot is found, add new bullet to vector
					if (!foundInactive)
						gs.bullets.push_back(bullet);
				}
			}
			else
			{
				obj.texture = tex;
				obj.currentAnimation = shootAnimIndex;
			}
		};

		switch (obj.data.playerData.state)
		{
			case PlayerState::idle:
			{
				if (currentDirection)
				{
					obj.data.playerData.state = PlayerState::running;
					
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

				handleShooting(rs.texIdle, rs.texShoot, rs.ANIM_PLAYER_IDLE, rs.ANIM_PLAYER_SHOOT);

				break;
			}
			case PlayerState::jumping:
			{
				handleShooting(rs.texRunning, rs.texRunShoot, rs.ANIM_PLAYER_RUNNING, rs.ANIM_PLAYER_RUNNING);

				break;
			}
			case PlayerState::running:
			{
				if (!currentDirection)
				{
					obj.data.playerData.state = PlayerState::idle;
				}

				// Adding sliding when changing direction
				if (obj.velocity.x * obj.direction < 0 && obj.grounded)
				{
					handleShooting(rs.texSlide, rs.texSlideShoot, rs.ANIM_PLAYER_SLIDE, rs.ANIM_PLAYER_SLIDESHOOT);
					obj.texture = rs.texSlide;
					obj.currentAnimation = rs.ANIM_PLAYER_SLIDE;
				}
				else
				{
					handleShooting(rs.texRunning, rs.texRunShoot, rs.ANIM_PLAYER_RUNNING, rs.ANIM_PLAYER_RUNNING);
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
	}
	else
		if (obj.type == ObjectType::bullet)
		{
			if (obj.position.x - gs.mapViewport.x < 0 || // Left side
					obj.position.x - gs.mapViewport.x > state.logW || // Right side
					obj.position.y - gs.mapViewport.y < 0 ||
					obj.position.y - gs.mapViewport.y > state.logH)
				obj.data.bulletData.state = BulletState::inactive;
		}
			

	if (currentDirection)
		obj.direction = currentDirection;

	// Add acceleration to velocity
	obj.velocity += currentDirection * obj.acceleration * deltaTime;

	// Clamp velocity to max speed
	if (std::abs(obj.velocity.x) > obj.maxSpeedX)
	{
		obj.velocity.x = obj.direction * obj.maxSpeedX;
	}

	// Add velocity to position
	obj.position += obj.velocity * deltaTime;

	// Handle collision detection
	bool foundGround = false;

	for(auto &layer : gs.layers)
	{
		for (GameObject &other : layer)
		{
			if (&obj != &other)
			{
				checkCollision(state, gs, rs, obj, other, deltaTime);

				if (other.type == ObjectType::level)
				{
					// Grounded sensor
					SDL_FRect sensor
					{
						.x = obj.position.x + obj.collider.x,
						.y = obj.position.y + obj.collider.y + obj.collider.h,
						.w = obj.collider.w,
						.h = 1
					};

					SDL_FRect ground
					{
						.x = other.position.x + other.collider.x,
						.y = other.position.y + other.collider.y,
						.w = other.collider.w,
						.h = other.collider.h
					};

					SDL_FRect intersection = { 0 };

					if (SDL_GetRectIntersectionFloat(&sensor, &ground, &intersection))
					{
						foundGround = true;
					}
				}
			}
		}
	}

	if (obj.grounded != foundGround)
	{
		obj.grounded = foundGround;

		if (foundGround && obj.type == ObjectType::player)
		{
			obj.data.playerData.state = PlayerState::running;
		}
	}
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
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 2, 2, 0, 0, 0, 2, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 0, 2, 0, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};

	short foregroundMap[MAP_ROWS][MAP_COLS] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		5, 0, 0, 5, 5, 5, 5, 5, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	short backgroundMap[MAP_ROWS][MAP_COLS] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		6, 0, 0, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	const auto loadMap = [&state, &gs, &res](short layer[MAP_ROWS][MAP_COLS])
	{
		const auto createObject = [&state](int row, int col, SDL_Texture* tex, ObjectType type)
		{
			GameObject obj;
			obj.type = type;
			obj.texture = tex;
			obj.position = glm::vec2(col * TILE_SIZE, state.logH - (MAP_ROWS - row) * TILE_SIZE);
			obj.collider = { .x = 0, .y = 0, .w = TILE_SIZE, .h = TILE_SIZE };

			return obj;
		};

		for (int row = 0; row < MAP_ROWS; row++)
		{
			for (int col = 0; col < MAP_COLS; col++)
			{
				switch (layer[row][col])
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
						player.grounded = true;
						player.collider = { .x = 11, .y = 6,
											.w = 10, .h = 26
						};
						gs.layers[LAYER_IDX_CHARACTERS].push_back(player);
						gs.playerIndex = gs.layers[LAYER_IDX_CHARACTERS].size() - 1;	// Set the player index to the last added character

						break;
					}
					case 5: // grass
					{
						GameObject obj = createObject(row, col, res.texGrass, ObjectType::level);
						gs.foregroundTiles.push_back(obj);

						break;
					}
					case 6: // brick
					{
						GameObject obj = createObject(row, col, res.texBrick, ObjectType::level);
						gs.backgroundTiles.push_back(obj);

						break;
					}
				}
			}
		}
	};
	
	loadMap(map);
	loadMap(backgroundMap);
	loadMap(foregroundMap);

	assert(gs.playerIndex != -1);	// Ensure that the player was created
}

void checkCollision(const SDLState& state, GameState& gs, Resources &res, GameObject &a, GameObject &b, float deltaTime)
{
	SDL_FRect rectA
	{
		.x = a.position.x + a.collider.x,
		.y = a.position.y + a.collider.y,
		.w = a.collider.w,
		.h = a.collider.h
	};

	SDL_FRect rectB
	{
		.x = b.position.x + b.collider.x,
		.y = b.position.y + b.collider.y,
		.w = b.collider.w,
		.h = b.collider.h
	};

	SDL_FRect intersection{ 0 };

	if (SDL_GetRectIntersectionFloat(&rectA, &rectB, &intersection))
	{
		// Found intersection, resolve collision here
		collisionResponse(state, gs, res, rectA, rectB, intersection, a, b, deltaTime);
	}
}

void collisionResponse(const SDLState& state, GameState& gs, Resources &res, const SDL_FRect &rectA, const SDL_FRect &rectB, 
						const SDL_FRect& intersection, GameObject &objA, GameObject &objB, float deltaTime)
{
	// Collision response logic goes here
	// Object we are checking
	if (objA.type == ObjectType::player)
	{
		// Object we are colliding with
		switch (objB.type)
		{
			case ObjectType::level:
			{
				if (intersection.w < intersection.h)
				{
					// Horizontal collision, resolve on the X axis
					if (objA.velocity.x > 0)
					{
						// Moving right, push back to the left
						objA.position.x -= intersection.w;
					}
					else 
						if (objA.velocity.x < 0)
						{
							// Moving left, push back to the right
							objA.position.x += intersection.w;
						}

					objA.velocity.x = 0;	// Stop horizontal movement on collision
				}
				else
				{
					// Vertical collision, resolve on the Y axis
					if (objA.velocity.y > 0)
					{
						// Moving right, push back to the left
						objA.position.y -= intersection.h;
					}
					else 
						if (objA.velocity.x < 0)
							{
								// Moving left, push back to the right
								objA.position.y += intersection.h;
							}

					objA.velocity.y = 0;	// Stop vertical movement on collision
				}
				break;
			}
		}
	}
}

void handleKeyInput(const SDLState &state, GameState &gs, GameObject &obj, SDL_Scancode key, bool keyDown)
{
	const float JUMP_FORCE = -200.0f;

	// Handle player input here
	if (obj.type == ObjectType::player)
	{
		switch (obj.data.playerData.state)
		{
			case PlayerState::idle:
			{
				if (key == SDL_SCANCODE_SPACE && keyDown)
				{
					obj.data.playerData.state = PlayerState::jumping;
					obj.velocity.y += JUMP_FORCE;
				}
				break;
			}
			case PlayerState::running:
			{
				if (key == SDL_SCANCODE_SPACE && keyDown)
				{
					obj.data.playerData.state = PlayerState::jumping;
					obj.velocity.y += JUMP_FORCE;
				}
				break;
			}
		}
	}
}

void drawParallaxBackground(SDL_Renderer* renderer, SDL_Texture* texture, float xVelocity,
	float& scrollPos, float scrollFactor, float deltaTime)
{
	scrollPos -= xVelocity * scrollFactor * deltaTime;

	if (scrollPos < -texture->w)
		scrollPos = 0;

	SDL_FRect dst =
	{
		.x = scrollPos,
		.y = 10,
		.w = texture->w * 2.0f,
		.h = static_cast<float>(texture->h)
	};

	SDL_RenderTextureTiled(renderer, texture, nullptr, 1, &dst);
}

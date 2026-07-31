#pragma once

#include<SDL3/SDL.h>
#include<glm/glm.hpp>
#include<vector>

#include "animation.h"

// Player's current movement/action state, driving both its update() logic
// and which animation/texture is shown.
enum class PlayerState
{
	idle,
	running,
	jumping,
	crouching
};

// Bullet's current lifecycle state.
enum class BulletState
{
	moving,		// Traveling in a straight line, checked for collisions and going off-screen
	colliding,	// Just hit something; playing its hit animation before deactivating
	inactive	// Not in play; eligible to be recycled by the bullet pool in GameState::bullets
};

// Enemy's current AI/health state.
enum class EnemyState
{
	shambling,	// Idle/chasing behavior
	damaged, 	// Briefly flashing/stunned after being hit, before returning to shambling
	dead		// Hit points depleted; playing death animation and no longer interactive
};

// Per-type data for a player GameObject. Only valid when GameObject::type == ObjectType::player.
struct PlayerData
{
	PlayerState state;
	Timer weaponTimer;	// Cooldown between shots while the fire key is held

	PlayerData() : weaponTimer(0.1f)
	{
		state = PlayerState::idle;
	}
};

// Per-type data for a level (static geometry) GameObject. Currently empty;
// exists so ObjectType::level has a matching ObjectData union member.
struct LevelData
{

};

// Per-type data for an enemy GameObject. Only valid when GameObject::type == ObjectType::enemy.
struct EnemyData
{
	EnemyState state;
	Timer damagedTimer;	// How long the enemy stays in the "damaged" (flashing) state after being hit
	int hitPoints;			// Remaining health; enemy dies when this drops to zero or below

	EnemyData() : state(EnemyState::shambling), damagedTimer(0.5f)
	{
		hitPoints = 100;
	}
};

// Per-type data for a bullet GameObject. Only valid when GameObject::type == ObjectType::bullet.
struct BulletData
{
	BulletState state;

	BulletData() : state(BulletState::moving)
	{

	}
};

/**
 * @brief Type-specific state for a GameObject.
 *
 * Only the member matching the owning GameObject's ObjectType is valid at
 * any given time -- e.g. a GameObject with type == ObjectType::enemy should
 * only have its `enemyData` member read/written, never `playerData` or
 * `bulletData`. GameObject's constructor default-initializes this as
 * `levelData` since ObjectType::level is the default type.
 */
union ObjectData
{
	PlayerData playerData;
	LevelData levelData;
	EnemyData enemyData;
	BulletData bulletData;
};

// Discriminates which member of ObjectData is active for a given GameObject,
// and which branch of update()/collisionResponse() applies to it.
enum class ObjectType
{
	player,
	level,
	enemy,
	bullet
};

/**
 * @brief A single entity in the game world.
 *
 * Every player, enemy, bullet, and piece of level geometry is represented
 * by this one struct, distinguished by `type`. Type-specific state (health,
 * AI/animation state machine, weapon cooldown, etc.) lives in the `data`
 * union -- see ObjectData. GameObjects are stored by value in GameState's
 * vectors and are updated/drawn each frame by update()/drawObject() in
 * SDLPlatformer.cpp.
 */
struct GameObject
{
	ObjectType type;	// The type of game object we're dealing with
	ObjectData data;	// Type-specific data; only the member matching `type` is valid
	glm::vec2 position;	// X and Y coordinates of the game object, in world space
	glm::vec2 velocity;	// Object's speed, in pixels/second
	glm::vec2 acceleration;	// Object's acceleration value, in pixels/second^2
	float direction;	// Direction in which the object is facing: 1 = right, -1 = left
	float maxSpeedX;	// Max speed in the X direction; velocity.x is clamped to +/- this each update
	std::vector<Animation> animations;	// Vector containing all animations for this GameObject
	int currentAnimation;	// Index into `animations` currently playing, or -1 for none (uses spriteFrame instead)
	SDL_Texture* texture;	// Currently loaded texture
	bool dynamic;		// Whether the object is dynamic (affected by physics) or static (not affected by physics)
	bool grounded;		// Whether the object is currently on the ground or in the air
	SDL_FRect collider;		// Collider rectangle for the object, relative to `position`
	Timer flashTimer;	// Duration of the damage-flash tint applied while shouldFlash is true
	bool shouldFlash;	// Whether to render this object with a reddish damage tint this frame
	int spriteFrame;	// Fixed sprite-sheet frame to draw when currentAnimation == -1

	/**
	 * @brief Default-constructs a static level-type GameObject at the
	 * origin with no texture/animation/collider. Callers (typically
	 * createTiles() in SDLPlatformer.cpp) are expected to overwrite `type`,
	 * `position`, `texture`, `collider`, and `data` as appropriate for the
	 * kind of object being created.
	 */
	GameObject() : data{ .levelData = LevelData() }, collider{ 0 }, flashTimer(0.05f) // Initialize the union with levelData by default
	{
		type = ObjectType::level;
		direction = 1;
		position = glm::vec2(0);
		velocity = glm::vec2(0);
		acceleration = glm::vec2(0);
		currentAnimation = -1;
		texture = nullptr;
		maxSpeedX = 0.0f;
		dynamic = false;
		grounded = false;
		shouldFlash = false;
		spriteFrame = 1;
	}
};
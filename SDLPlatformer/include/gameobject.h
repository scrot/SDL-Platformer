#pragma once

#include<SDL3/SDL.h>
#include<glm/glm.hpp>
#include<vector>

#include "animation.h"

enum class PlayerState
{
	idle, jumping, running
};

struct PlayerData
{
	PlayerState state;
	PlayerData()
	{
		state = PlayerState::idle;
	}
};

struct LevelData
{

};

struct EnemyData
{

};

union ObjectData
{
	PlayerData playerData;
	LevelData levelData;
	EnemyData enemyData;
};

enum class ObjectType
{
	player,
	level,
	enemy
};

struct GameObject
{
	ObjectType type;	// The type of game object we're dealing with
	ObjectData data;	// Data about the object
	glm::vec2 position;	// X and Y coordinates of the game object
	glm::vec2 velocity;	// Object's speed
	glm::vec2 acceleration;	// Object's acceleration value
	float direction;	// Direction in which the object is facing
	float maxSpeedX;	// Max speed in the X direction
	std::vector<Animation> animations;	// Vector containing all animations for this GameObject
	int currentAnimation;	// Animation that's currently playing
	SDL_Texture* texture;	// Currently loaded texture
	
	GameObject() : data{ .levelData = LevelData() } // Initialize the union with levelData by default
	{
		type = ObjectType::level;
		direction = 1;
		position = glm::vec2(0);
		velocity = glm::vec2(0);
		acceleration = glm::vec2(0);
		currentAnimation = -1;
		texture = nullptr;
		maxSpeedX = 0.0f;
	}
};
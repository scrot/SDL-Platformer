#pragma once

#include<SDL3/SDL.h>
#include<glm/glm.hpp>
#include<vector>

#include "animation.h"

enum class ObjectType
{
	player,
	level,
	enemy
};

struct GameObject
{
	ObjectType type;
	glm::vec2 position;
	glm::vec2 velocity;
	glm::vec2 acceleration;
	float direction;
	std::vector<Animation> animations;
	int currentAnimation;
	SDL_Texture* texture;
	
	GameObject()
	{
		type = ObjectType::level;
		direction = 1;
		position = glm::vec2(0);
		velocity = glm::vec2(0);
		acceleration = glm::vec2(0);
		currentAnimation = -1;
		texture = nullptr;
	}
};
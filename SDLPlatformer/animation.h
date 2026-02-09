#pragma once

#include "timer.h"

class Animation
{
	Timer timer;
	int frameCount;

public:
	Animation()
	{
		timer = 0.0f;
		frameCount = 0;
	}

	Animation(int frameCount, float length)
	{
		this->frameCount = frameCount;
		this->timer = Timer(length);
	}

	const float getLength()
	{
		return timer.getLength();
	}

	const int currentFrame()
	{
		return static_cast<int> (timer.getTime() / timer.getLength() * frameCount);
	}

	void step(float deltaTime)
	{
		timer.step(deltaTime);
	}
};
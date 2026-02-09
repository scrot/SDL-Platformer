#pragma once

class Timer
{
	float length;
	float time;
	bool timeout;

public:
	Timer() {}

	Timer(float length)
	{
		this->length = length;
		this->time = 0;
		timeout = false;
	}

	void step(float deltaTime)
	{
		time += deltaTime;

		if (time >= length)
		{
			time -= length;
			timeout = true;
		}
	}

	const bool isTimeout()
	{
		return timeout;
	}

	const float getTime()
	{
		return time;
	}

	const float getLength()
	{
		return length;
	}

	void reset()
	{
		time = 0;
		timeout = false;
	}
};
#pragma once

class Timer
{
	float length;
	float time;

public:
	Timer(float length)
	{
		this->length = length;
		this->time = 0;
	}
};
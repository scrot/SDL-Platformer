#pragma once

#include "timer.h"

/**
 * @brief A frame-based sprite-sheet animation.
 *
 * Wraps a Timer covering the animation's total playback length and derives
 * the current frame index from the fraction of that length elapsed. Frames
 * are assumed to be laid out left-to-right in a single row of a sprite
 * sheet, each the same fixed width (see drawObject() in SDLPlatformer.cpp).
 */
class Animation
{
	Timer timer;		// Drives playback progress over the animation's total length
	int frameCount;		// Number of frames in the sprite sheet row

public:
	Animation()
	{
		timer = 0.0f;
		frameCount = 0;
	}

	/**
	* @brief Initializes a new Animation object.
	*
	* @param frameCount -- The number of frames in the animation.
	* @param length -- The total length of the animation.
	*/
	Animation(int frameCount, float length)
	{
		this->frameCount = frameCount;
		this->timer = Timer(length);
	}

	/**
	 * @brief The total playback length of the animation, in seconds.
	 */
	const float getLength()
	{
		return timer.getLength();
	}

	/**
	 * @brief The index of the frame that should currently be displayed,
	 * derived from how far through the animation's length the timer is.
	 */
	const int currentFrame()
	{
		return static_cast<int> (timer.getTime() / timer.getLength() * frameCount);
	}

	/**
	 * @brief Advances animation playback by the given amount of time.
	 *
	 * @param deltaTime -- Time elapsed since the last step, in seconds.
	 */
	void step(float deltaTime)
	{
		timer.step(deltaTime);
	}

	/**
	 * @brief The amount of time elapsed in the current playback cycle, in seconds.
	 */
	float getTime()
	{
		return timer.getTime();
	}

	/**
	 * @brief Whether the animation has completed a full cycle since it last looped/reset.
	 */
	const bool isDone()
	{
		return timer.isTimeout();
	}
};
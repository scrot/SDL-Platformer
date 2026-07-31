#pragma once

/**
 * @brief A simple repeating countdown/stopwatch helper.
 *
 * Accumulates elapsed time via step() and wraps back to zero every time it
 * reaches its configured length, flagging that a timeout occurred. Used to
 * drive animation frame timing, weapon cooldowns, and other timed states.
 */
class Timer
{
	float length;	// Duration in seconds before the timer times out
	float time;		// Elapsed time in seconds since the last reset/wrap
	bool timeout;	// Whether the timer has timed out since it was last reset

public:
	Timer() {}

	/**
	 * @brief Constructs a timer with the given duration.
	 *
	 * @param length -- The duration in seconds before the timer times out.
	 */
	Timer(float length)
	{
		this->length = length;
		this->time = 0;
		timeout = false;
	}

	/**
	 * @brief Advances the timer by the given amount of time.
	 *
	 * If the accumulated time reaches the timer's length, it wraps back
	 * around (carrying over any excess) and marks the timer as timed out.
	 *
	 * @param deltaTime -- Time elapsed since the last step, in seconds.
	 *
	 * @return true if the timer reached its length this step, false otherwise.
	 */
	bool step(float deltaTime)
	{
		time += deltaTime;

		if (time >= length)
		{
			time -= length;
			timeout = true;

			return true;
		}

		return false;
	}

	/**
	 * @brief Whether the timer has timed out since it was last reset.
	 */
	const bool isTimeout()
	{
		return timeout;
	}

	/**
	 * @brief The amount of time elapsed since the timer was last reset/wrapped, in seconds.
	 */
	const float getTime()
	{
		return time;
	}

	/**
	 * @brief The configured duration of the timer, in seconds.
	 */
	const float getLength()
	{
		return length;
	}

	/**
	 * @brief Resets the elapsed time and timeout flag back to their initial state.
	 */
	void reset()
	{
		time = 0;
		timeout = false;
	}
};
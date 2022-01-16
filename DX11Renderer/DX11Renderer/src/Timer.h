#pragma once
#include <chrono>

namespace gfx
{
	class Timer
	{
	public:
		Timer();
		float Mark();
		float Peek() const;
	private:
		std::chrono::steady_clock::time_point last;
	};
}
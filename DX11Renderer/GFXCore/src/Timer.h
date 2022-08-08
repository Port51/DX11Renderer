#pragma once
#include <chrono>

namespace gfxcore
{
	class Timer
	{
	public:
		Timer();
		virtual ~Timer();
		float Mark();
		float Peek() const;
	private:
		std::chrono::steady_clock::time_point last;
	};
}
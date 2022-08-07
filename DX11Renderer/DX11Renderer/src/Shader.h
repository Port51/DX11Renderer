#pragma once
#include "Bindable.h"

namespace gfx
{
	class Shader : public Bindable
	{
	protected:
		const bool PathEndsWithCSO(const char* path) const;
	};
}
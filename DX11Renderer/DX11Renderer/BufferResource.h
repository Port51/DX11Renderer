#pragma once

class Graphics;

namespace Bind
{
	class BufferResource
	{
	public:
		virtual ~BufferResource() = default;
		virtual void BindAsBuffer(Graphics&) = 0;
		virtual void BindAsBuffer(Graphics&, BufferResource*) = 0;
		virtual void Clear(Graphics&) = 0;
	};
}
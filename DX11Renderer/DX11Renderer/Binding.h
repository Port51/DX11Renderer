#pragma once
#include <string>
#include "WindowsInclude.h"

class Graphics;

namespace Bind
{
	class Bindable;

	class Binding
	{
	public:
		Binding(std::shared_ptr<Bindable> _pBindable, UINT _slot);
		~Binding() = default;
		void Bind(Graphics& gfx) const;
		std::shared_ptr<Bindable> GetBindable() const;
	private:
		std::shared_ptr<Bindable> pBindable;
		UINT slot;
	};
}
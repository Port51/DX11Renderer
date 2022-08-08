#include "CommonHeader.h"

namespace gfx
{
	class Binding;
	class Bindable;
	class GraphicsDevice;

	class BindingList
	{
	public:
		BindingList();

	public:
		Binding& AddBinding(const Binding binding);
		Binding& AddBinding(std::shared_ptr<Bindable> pBindable);
		void Bind(const GraphicsDevice& gfx, RenderState& renderState) const;

	private:
		std::vector<Binding> m_bindings;

	};
}
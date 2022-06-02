#pragma once
#include "CommonHeader.h"

namespace gfx
{
	class GraphicsDevice;
	class Bindable;
	class RenderPass;

	class Binding
	{
	public:
		Binding(std::shared_ptr<Bindable> pBindable);
		virtual ~Binding() = default;
	public:
		void Bind(const GraphicsDevice& gfx, RenderState& renderState) const;
		void Unbind(const GraphicsDevice& gfx, RenderState& renderState) const;
		const Bindable& GetBindable() const;
		Binding& SetupIABinding(const slotUINT slot = 0u);
		Binding& SetupCSBinding(const slotUINT slot);
		Binding& SetupVSBinding(const slotUINT slot);
		Binding& SetupPSBinding(const slotUINT slot);
		Binding& SetupRSBinding();
		Binding& SetupOMBinding();
	private:
		const std::shared_ptr<Bindable> m_pBindable;
		int m_IASlot = -1;
		int m_CSSlot = -1;
		int m_VSSlot = -1;
		int m_PSSlot = -1;
		bool m_RSBind = false;
		bool m_OMBind = false;
	};
}
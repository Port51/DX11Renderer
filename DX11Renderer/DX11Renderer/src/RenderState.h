#pragma once
#include "CommonHeader.h"
#include "RenderConstants.h"
#include <unordered_map>

namespace gfx
{

	// Helper class for minimizing state changes during draw calls
	class RenderState
	{
	public:
		RenderState();
	public:
		// Returns TRUE if this is a new binding, and stores that binding
		// Use this to tell whether to actually bind a resource or not
		const bool IsNewBinding(const guid64 guid, const RenderBindingType::RenderBindingTypeEnum bindingType, const slotUINT slot);
		const void ClearBinding(const RenderBindingType::RenderBindingTypeEnum bindingType, const slotUINT slot);
	private:
		const u16 GetKey(const RenderBindingType::RenderBindingTypeEnum bindingType, const slotUINT slot) const;
	private:
		// Bindings are stored according to bits: [ 8 bit binding identifier, 8 bit slot ]
		// The binding identifier represents things like "CS SRV", "Index Buffer", etc.
		std::unordered_map<u16, guid64> m_activeBindings;
	};

}
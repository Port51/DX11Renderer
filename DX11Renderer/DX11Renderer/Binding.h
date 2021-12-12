#pragma once
#include "WindowsInclude.h"
#include <memory>

class Graphics;
class Bindable;
class RenderPass;

class Binding
{
public:
	Binding(std::shared_ptr<Bindable> _pBindable);
	~Binding() = default;
public:
	void Bind(Graphics& gfx) const;
	std::shared_ptr<Bindable> GetBindable() const;
	Binding& SetupIABinding(UINT slot = 0u);
	Binding& SetupCSBinding(UINT slot);
	Binding& SetupVSBinding(UINT slot);
	Binding& SetupPSBinding(UINT slot);
	Binding& SetupRSBinding();
	Binding& SetupOMBinding();
private:
	std::shared_ptr<Bindable> pBindable;
	int bindToInputAssemblerStageSlot = -1;
	int bindToComputeStageSlot = -1;
	int bindToVertexStageSlot = -1;
	int bindToPixelStageSlot = -1;
	bool bindToRasterizerStageSlot = false;
	bool bindToOutputMergerStageSlot = false;
};

#pragma once
#include <memory>
#include <vector>
#include <string_view>
#include "VertexLayout.h"
#include "Bindable.h"
#include "Technique.h"

class Technique;
class VertexShader;
class PixelShader;
class Material;
class FrameCommander;
class Binding;

///
/// Collection of bindables needed to execute a material pass in a technique
///
class MaterialPass
{
public:
	MaterialPass();
public:
	void AddTechnique(std::unique_ptr<Technique> _pTechnique);
	void AddBinding(std::shared_ptr<Bindable> pBindable, UINT slot = 0u);
	void AddBinding(Binding binding);
	void Bind(Graphics& gfx);
	void SubmitDrawCalls(FrameCommander& frame, const MeshRenderer& renderer) const;
public:
	std::shared_ptr<VertexShader> pVertexShader;
	std::shared_ptr<PixelShader> pPixelShader;
private:
	std::vector<Binding> bindings;
	Material* pParent;
	std::unique_ptr<Technique> pTechnique;
};
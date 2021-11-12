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

///
/// Collection of bindables needed to execute a material pass in a technique
///
class MaterialPass
{
public:
	MaterialPass();
public:
	void AddTechnique(std::unique_ptr<Technique> _pTechnique);
	void AddBindable(std::shared_ptr<Bindable> pBindable);
	void Bind(Graphics& gfx);
	void SubmitDrawCalls(FrameCommander& frame, const MeshRenderer& renderer) const;
public:
	std::shared_ptr<VertexShader> pVertexShader;
	std::shared_ptr<PixelShader> pPixelShader;
private:
	std::vector<std::shared_ptr<Bindable>> pBindables; // non-shader bindables
	Material* pParent;
	std::unique_ptr<Technique> pTechnique;
};
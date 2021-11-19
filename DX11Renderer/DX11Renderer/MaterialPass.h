#pragma once
#include <memory>

class Technique;
class MeshRenderer;
class FrameCommander;
class Binding;
//class VertexShader;
//class PixelShader;

///
/// Collection of bindables needed to execute a material pass in a technique
///
class MaterialPass
{
public:
	MaterialPass();
public:
	void AddTechnique(std::unique_ptr<Technique> _pTechnique);
	void SubmitDrawCalls(std::unique_ptr<FrameCommander>& frame, const MeshRenderer& renderer) const;
//public:
//	std::shared_ptr<VertexShader> pVertexShader;
//	std::shared_ptr<PixelShader> pPixelShader;
private:
	std::unique_ptr<Technique> pTechnique;
};
#pragma once
#include "Graphics.h"
#include "RenderJob.h"
#include <vector>

///
/// Wrapper containing jobs
///
class RenderPass
{
public:
	RenderPass()
	{

	}
public:
	void EnqueueJob(RenderJob job);
	virtual void BindGlobals(Graphics& gfx) const;
	virtual void Execute(Graphics& gfx) const;
	void Reset();
public:
	void AppendGlobalVSTextureBind(ID3D11ShaderResourceView* pSRV);
	void AppendGlobalPSTextureBind(ID3D11ShaderResourceView* pSRV);
	UINT GetVSTextureSlotOffset() const;
	UINT GetPSTextureSlotOffset() const;
private:
	std::vector<RenderJob> jobs; // will be replaced by render graph

	// Global binds
	std::vector<ID3D11ShaderResourceView*> pGlobalVSTextureBinds;
	std::vector<ID3D11ShaderResourceView*> pGlobalPSTextureBinds;
};
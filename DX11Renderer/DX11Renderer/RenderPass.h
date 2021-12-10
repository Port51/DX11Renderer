#pragma once
#include "Graphics.h"
#include "RenderJob.h"
#include <vector>
#include "CommonCbuffers.h"
#include "Common.h"

///
/// Wrapper containing jobs
///
class RenderPass
{
public:
	RenderPass()
	{

	}
	RenderPass(const RenderPass& parentPass);
public:
	void EnqueueJob(RenderJob job);
	virtual void BindSharedResources(Graphics& gfx) const;
	virtual void Execute(Graphics& gfx) const;
	void Reset();
public:
	RenderPass& CSAppendCB(ID3D11Buffer* pCB);
	RenderPass& CSAppendSRV(ID3D11ShaderResourceView* pSRV);
	RenderPass& CSAppendUAV(ID3D11UnorderedAccessView* pUAV);
	RenderPass& VSAppendCB(ID3D11Buffer* pCB);
	RenderPass& VSAppendSRV(ID3D11ShaderResourceView* pSRV);
	RenderPass& PSAppendCB(ID3D11Buffer* pCB);
	RenderPass& PSAppendSRV(ID3D11ShaderResourceView* pSRV);
	const BindSlots& GetStartSlots() const;
	const BindSlots& GetEndSlots() const;
private:
	std::vector<RenderJob> jobs; // will be replaced by render graph

	BindSlots startSlots;
	BindSlots endSlots;

	// Binds shared by everything in this render pass
	std::vector<ID3D11Buffer*> pCS_CB_Binds;
	std::vector<ID3D11ShaderResourceView*> pCS_SRV_Binds;
	std::vector<ID3D11UnorderedAccessView*> pCS_UAV_Binds;
	std::vector<ID3D11Buffer*> pVS_CB_Binds;
	std::vector<ID3D11ShaderResourceView*> pVS_SRV_Binds;
	std::vector<ID3D11Buffer*> pPS_CB_Binds;
	std::vector<ID3D11ShaderResourceView*> pPS_SRV_Binds;
};
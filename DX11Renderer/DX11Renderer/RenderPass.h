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
	virtual ~RenderPass() = default;
public:
	void EnqueueJob(RenderJob job);
	virtual void BindSharedResources(Graphics& gfx) const;
	virtual void Execute(Graphics& gfx) const;
	void Reset();
public:
	RenderPass& CSSetCB(UINT slot, ID3D11Buffer* pResource);
	RenderPass& CSSetSRV(UINT slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pResource);
	RenderPass& CSSetUAV(UINT slot, ID3D11UnorderedAccessView* pResource);
	RenderPass& VSSetCB(UINT slot, ID3D11Buffer* pResource);
	RenderPass& VSSetSRV(UINT slot, ID3D11ShaderResourceView* pResource);
	RenderPass& PSSetCB(UINT slot, ID3D11Buffer* pResource);
	RenderPass& PSSetSRV(UINT slot, ID3D11ShaderResourceView* pResource);
	const BindSlots& GetStartSlots() const;
	const BindSlots& GetEndSlots() const;
private:
	std::vector<RenderJob> jobs; // will be replaced by render graph

	BindSlots startSlots;
	BindSlots endSlots;

	// Binds shared by everything in this render pass
	std::vector<std::pair<UINT, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>> pCS_SRV_Binds;
	std::vector<ID3D11Buffer*> pCS_CB_Binds;
	//std::vector<ID3D11ShaderResourceView*> pCS_SRV_Binds;
	std::vector<ID3D11UnorderedAccessView*> pCS_UAV_Binds;
	std::vector<ID3D11Buffer*> pVS_CB_Binds;
	std::vector<ID3D11ShaderResourceView*> pVS_SRV_Binds;
	std::vector<ID3D11Buffer*> pPS_CB_Binds;
	std::vector<ID3D11ShaderResourceView*> pPS_SRV_Binds;
};
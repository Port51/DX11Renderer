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
	RenderPass();
	virtual ~RenderPass() = default;
public:
	void EnqueueJob(RenderJob job);
	virtual void BindSharedResources(Graphics& gfx) const;
	virtual void UnbindSharedResources(Graphics& gfx) const;
	virtual void Execute(Graphics& gfx) const;
	void Reset();
public:
	RenderPass& CSSetCB(UINT slot, Microsoft::WRL::ComPtr<ID3D11Buffer> pResource);
	RenderPass& CSSetSRV(UINT slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pResource);
	RenderPass& CSSetUAV(UINT slot, Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pResource);
	RenderPass& VSSetCB(UINT slot, Microsoft::WRL::ComPtr<ID3D11Buffer> pResource);
	RenderPass& VSSetSRV(UINT slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pResource);
	RenderPass& PSSetCB(UINT slot, Microsoft::WRL::ComPtr<ID3D11Buffer> pResource);
	RenderPass& PSSetSRV(UINT slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pResource);
private:
	std::vector<RenderJob> jobs; // will be replaced by render graph

	// Binds shared by everything in this render pass
	std::vector<std::pair<UINT, Microsoft::WRL::ComPtr<ID3D11Buffer>>> pCS_CB_Binds;
	std::vector<std::pair<UINT, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>> pCS_SRV_Binds;
	std::vector<std::pair<UINT, Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>>> pCS_UAV_Binds;
	std::vector<std::pair<UINT, Microsoft::WRL::ComPtr<ID3D11Buffer>>> pVS_CB_Binds;
	std::vector<std::pair<UINT, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>> pVS_SRV_Binds;
	std::vector<std::pair<UINT, Microsoft::WRL::ComPtr<ID3D11Buffer>>> pPS_CB_Binds;
	std::vector<std::pair<UINT, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>> pPS_SRV_Binds;

	static std::vector<ID3D11Buffer*> pNullBuffers;
	static std::vector<ID3D11ShaderResourceView*> pNullSRVs;
	static std::vector<ID3D11UnorderedAccessView*> pNullUAVs;
};
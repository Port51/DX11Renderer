#pragma once
#include "Common.h"
#include "Graphics.h"
#include "DrawCall.h"
#include <vector>
#include "CommonCbuffers.h"

class Binding;
class Bindable;

///
/// Wrapper containing jobs
///
class RenderPass
{
public:
	RenderPass(std::string name);
	virtual ~RenderPass() = default;
public:
	const std::string GetName() const;
	void EnqueueJob(DrawCall job);
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
	Binding& AddBinding(std::shared_ptr<Bindable> pBindable);
	Binding& AddBinding(Binding pBinding);
protected:
	std::vector<Binding> bindings;
private:
	const std::string name;
	std::vector<DrawCall> jobs; // will be replaced by render graph

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
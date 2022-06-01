#include "pch.h"
#include "Sampler.h"
#include "SharedCodex.h"
#include "GraphicsDevice.h"

namespace gfx
{
	Sampler::Sampler(const GraphicsDevice& gfx)
		: Sampler(gfx, D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP)
	{}

	Sampler::Sampler(const GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU)
		: Sampler(gfx, D3D11_FILTER_ANISOTROPIC, wrapU, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP)
	{}

	Sampler::Sampler(const GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV)
		: Sampler(gfx, D3D11_FILTER_ANISOTROPIC, wrapU, wrapV, D3D11_TEXTURE_ADDRESS_WRAP)
	{}

	Sampler::Sampler(const GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW)
		: Sampler(gfx, D3D11_FILTER_ANISOTROPIC, wrapU, wrapV, D3D11_TEXTURE_ADDRESS_WRAP)
	{}

	Sampler::Sampler(const GraphicsDevice& gfx, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW)
		: m_filter(filter), m_wrapU(wrapU), m_wrapV(wrapV), m_wrapW(wrapW)
	{
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = filter;
		samplerDesc.AddressU = wrapU;
		samplerDesc.AddressV = wrapV;
		samplerDesc.AddressW = wrapW;
		samplerDesc.MipLODBias = 0.f;
		samplerDesc.MinLOD = 0.f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

		// todo: add color option in constructor
		samplerDesc.BorderColor[0] = 1.0f;
		samplerDesc.BorderColor[1] = 1.0f;
		samplerDesc.BorderColor[2] = 1.0f;
		samplerDesc.BorderColor[3] = 1.0f;

		THROW_IF_FAILED(gfx.GetAdapter()->CreateSamplerState(&samplerDesc, &m_pSampler));
	}

	Sampler::Sampler(const GraphicsDevice& gfx, D3D11_SAMPLER_DESC samplerDesc)
	{
		THROW_IF_FAILED(gfx.GetAdapter()->CreateSamplerState(&samplerDesc, &m_pSampler));
	}

	void Sampler::Release()
	{
		m_pSampler.Reset();
	}

	void Sampler::BindCS(const GraphicsDevice& gfx, RenderState& renderState, UINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::CS_Sampler, slot))
		{
			gfx.GetContext()->CSSetSamplers(slot, 1u, m_pSampler.GetAddressOf());
		}
	}

	void Sampler::UnbindCS(const GraphicsDevice & gfx, RenderState & renderState, UINT slot)
	{
		renderState.ClearBinding(RenderBindingType::CS_Sampler, slot);
		gfx.GetContext()->CSSetSamplers(slot, 1u, nullptr);
	}

	void Sampler::BindVS(const GraphicsDevice& gfx, RenderState& renderState, UINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::VS_Sampler, slot))
		{
			gfx.GetContext()->VSSetSamplers(slot, 1u, m_pSampler.GetAddressOf());
		}
	}

	void Sampler::UnbindVS(const GraphicsDevice & gfx, RenderState & renderState, UINT slot)
	{
		renderState.ClearBinding(RenderBindingType::VS_Sampler, slot);
		gfx.GetContext()->VSSetSamplers(slot, 1u, nullptr);
	}

	void Sampler::BindPS(const GraphicsDevice& gfx, RenderState& renderState, UINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::PS_Sampler, slot))
		{
			gfx.GetContext()->PSSetSamplers(slot, 1u, m_pSampler.GetAddressOf());
		}
	}

	void Sampler::UnbindPS(const GraphicsDevice & gfx, RenderState & renderState, UINT slot)
	{
		renderState.ClearBinding(RenderBindingType::PS_Sampler, slot);
		gfx.GetContext()->PSSetSamplers(slot, 1u, RenderConstants::NullSamplerArray.data());
	}

	const ComPtr<ID3D11SamplerState> Sampler::GetD3DSampler() const
	{
		return m_pSampler;
	}

	std::shared_ptr<Bindable> Sampler::Resolve(const GraphicsDevice& gfx)
	{
		return std::move(Resolve(gfx, D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP));
	}

	std::shared_ptr<Bindable> Sampler::Resolve(const GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU)
	{
		return std::move(Resolve(gfx, D3D11_FILTER_ANISOTROPIC, wrapU, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP));
	}

	std::shared_ptr<Bindable> Sampler::Resolve(const GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV)
	{
		return std::move(Resolve(gfx, D3D11_FILTER_ANISOTROPIC, wrapU, wrapV, D3D11_TEXTURE_ADDRESS_WRAP));
	}

	std::shared_ptr<Bindable> Sampler::Resolve(const GraphicsDevice& gfx, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW)
	{
		return std::move(Resolve(gfx, D3D11_FILTER_ANISOTROPIC, wrapU, wrapV, D3D11_TEXTURE_ADDRESS_WRAP));
	}

	std::shared_ptr<Bindable> Sampler::Resolve(const GraphicsDevice& gfx, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW)
	{
		return std::move(Codex::Resolve<Sampler>(gfx, GenerateUID(filter, wrapU, wrapV, wrapW), wrapU, wrapV, wrapW));
	}

	std::string Sampler::GenerateUID(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE wrapU, D3D11_TEXTURE_ADDRESS_MODE wrapV, D3D11_TEXTURE_ADDRESS_MODE wrapW)
	{
		return std::string(typeid(Sampler).name()) + "|"
			+ std::to_string(static_cast<uint32_t>(filter))
			+ std::to_string(static_cast<uint32_t>(wrapU))
			+ std::to_string(static_cast<uint32_t>(wrapV))
			+ std::to_string(static_cast<uint32_t>(wrapW));
	}
}
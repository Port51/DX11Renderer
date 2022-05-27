#include "pch.h"
#include <functional>
#include "RenderPass.h"
#include "Binding.h"
#include "Bindable.h"
#include "RenderTexture.h"
#include "RenderState.h"

namespace gfx
{
	// Statics
	std::vector<ID3D11Buffer*> RenderPass::m_pNullBuffers;
	std::vector<ID3D11ShaderResourceView*> RenderPass::m_pNullSRVs;
	std::vector<ID3D11UnorderedAccessView*> RenderPass::m_pNullUAVs;
	std::vector<ID3D11SamplerState*> RenderPass::m_pNullSPLs;

	RenderPass::RenderPass(const RenderPassType renderPassType)
		: m_renderPassType(renderPassType)
	{
		if (m_pNullBuffers.size() == 0)
		{
			// Init static info
			m_pNullBuffers.resize(10u, nullptr);
			m_pNullSRVs.resize(10u, nullptr);
			m_pNullUAVs.resize(10u, nullptr);
			m_pNullSPLs.resize(10u, nullptr);
		}
	}

	const RenderPassType RenderPass::GetRenderPassType() const
	{
		return m_renderPassType;
	}

	void RenderPass::EnqueueJob(DrawCall job)
	{
		m_jobs.push_back(std::move(job));
	}

	void RenderPass::BindSharedResources(const GraphicsDevice& gfx) const
	{
		for (auto& binding : m_bindings)
		{
			binding.Bind(gfx);
		}

		// todo: can optimize by passing one array for each of these
		for (size_t i = 0; i < m_CS_CB_Binds.size(); ++i)
		{
			gfx.GetContext()->CSSetConstantBuffers(m_CS_CB_Binds[i].first, 1u, m_CS_CB_Binds[i].second.GetAddressOf());
		}
		for (size_t i = 0; i < m_CS_SRV_Binds.size(); ++i)
		{
			gfx.GetContext()->CSSetShaderResources(m_CS_SRV_Binds[i].first, 1u, m_CS_SRV_Binds[i].second.GetAddressOf());
		}
		for (size_t i = 0; i < m_CS_UAV_Binds.size(); ++i)
		{
			gfx.GetContext()->CSSetUnorderedAccessViews(m_CS_UAV_Binds[i].first, 1u, m_CS_UAV_Binds[i].second.GetAddressOf(), nullptr);
		}
		for (size_t i = 0; i < m_CS_SPL_Binds.size(); ++i)
		{
			gfx.GetContext()->CSSetSamplers(m_CS_SPL_Binds[i].first, 1u, m_CS_SPL_Binds[i].second.GetAddressOf());
		}

		for (size_t i = 0; i < m_VS_CB_Binds.size(); ++i)
		{
			gfx.GetContext()->VSSetConstantBuffers(m_VS_CB_Binds[i].first, 1u, m_VS_CB_Binds[i].second.GetAddressOf());
		}
		for (size_t i = 0; i < m_VS_SRV_Binds.size(); ++i)
		{
			gfx.GetContext()->VSSetShaderResources(m_VS_SRV_Binds[i].first, 1u, m_VS_SRV_Binds[i].second.GetAddressOf());
		}
		for (size_t i = 0; i < m_VS_SPL_Binds.size(); ++i)
		{
			gfx.GetContext()->VSSetSamplers(m_VS_SPL_Binds[i].first, 1u, m_VS_SPL_Binds[i].second.GetAddressOf());
		}

		for (size_t i = 0; i < m_PS_CB_Binds.size(); ++i)
		{
			gfx.GetContext()->PSSetConstantBuffers(m_PS_CB_Binds[i].first, 1u, m_PS_CB_Binds[i].second.GetAddressOf());
		}
		for (size_t i = 0; i < m_PS_SRV_Binds.size(); ++i)
		{
			gfx.GetContext()->PSSetShaderResources(m_PS_SRV_Binds[i].first, 1u, m_PS_SRV_Binds[i].second.GetAddressOf());
		}
		for (size_t i = 0; i < m_PS_SPL_Binds.size(); ++i)
		{
			gfx.GetContext()->PSSetSamplers(m_PS_SPL_Binds[i].first, 1u, m_PS_SPL_Binds[i].second.GetAddressOf());
		}
	}

	void RenderPass::UnbindSharedResources(const GraphicsDevice& gfx) const
	{
		if (m_CS_CB_Binds.size() > 0)
		{
			gfx.GetContext()->CSSetConstantBuffers(m_CS_CB_Binds[0].first, m_CS_CB_Binds.size(), m_pNullBuffers.data());
		}
		if (m_CS_SRV_Binds.size() > 0)
		{
			gfx.GetContext()->CSSetShaderResources(m_CS_SRV_Binds[0].first, m_CS_SRV_Binds.size(), m_pNullSRVs.data());
		}
		if (m_CS_UAV_Binds.size() > 0)
		{
			gfx.GetContext()->CSSetUnorderedAccessViews(m_CS_UAV_Binds[0].first, m_CS_UAV_Binds.size(), m_pNullUAVs.data(), nullptr);
		}
		if (m_CS_SPL_Binds.size() > 0)
		{
			gfx.GetContext()->CSSetSamplers(m_CS_SPL_Binds[0].first, m_CS_SPL_Binds.size(), m_pNullSPLs.data());
		}

		if (m_VS_CB_Binds.size() > 0)
		{
			gfx.GetContext()->VSSetConstantBuffers(m_VS_CB_Binds[0].first, m_VS_CB_Binds.size(), m_pNullBuffers.data());
		}
		if (m_VS_SRV_Binds.size() > 0)
		{
			gfx.GetContext()->VSSetShaderResources(m_VS_SRV_Binds[0].first, m_VS_SRV_Binds.size(), m_pNullSRVs.data());
		}
		if (m_VS_SPL_Binds.size() > 0)
		{
			gfx.GetContext()->VSSetSamplers(m_VS_SPL_Binds[0].first, m_VS_SPL_Binds.size(), m_pNullSPLs.data());
		}

		if (m_PS_CB_Binds.size() > 0)
		{
			gfx.GetContext()->PSSetConstantBuffers(m_PS_CB_Binds[0].first, m_PS_CB_Binds.size(), m_pNullBuffers.data());
		}
		if (m_PS_SRV_Binds.size() > 0)
		{
			gfx.GetContext()->PSSetShaderResources(m_PS_SRV_Binds[0].first, m_PS_SRV_Binds.size(), m_pNullSRVs.data());
		}
		if (m_PS_SPL_Binds.size() > 0)
		{
			gfx.GetContext()->PSSetSamplers(m_PS_SPL_Binds[0].first, m_PS_SPL_Binds.size(), m_pNullSPLs.data());
		}
	}

	void RenderPass::Execute(const GraphicsDevice& gfx) const
	{
		RenderState renderState;
		for (const auto& j : m_jobs)
		{
			j.Execute(gfx, renderState);
		}
	}

	void RenderPass::Reset()
	{
		m_jobs.clear();
	}

	void RenderPass::DrawImguiControls(const GraphicsDevice & gfx)
	{
	}

	RenderPass& RenderPass::ClearBinds()
	{
		m_CS_CB_Binds.clear();
		m_CS_SRV_Binds.clear();
		m_CS_UAV_Binds.clear();
		m_CS_SPL_Binds.clear();
		m_VS_CB_Binds.clear();
		m_VS_SRV_Binds.clear();
		m_VS_SPL_Binds.clear();
		m_PS_CB_Binds.clear();
		m_PS_SRV_Binds.clear();
		m_PS_SPL_Binds.clear();
		return *this;
	}

	RenderPass& RenderPass::CSSetCB(UINT slot, ComPtr<ID3D11Buffer> pResource)
	{
		m_CS_CB_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11Buffer>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::CSSetSRV(UINT slot, ComPtr<ID3D11ShaderResourceView> pResource)
	{
		m_CS_SRV_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11ShaderResourceView>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::CSSetUAV(UINT slot, ComPtr<ID3D11UnorderedAccessView> pResource)
	{
		m_CS_UAV_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11UnorderedAccessView>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::CSSetSPL(UINT slot, ComPtr<ID3D11SamplerState> pResource)
	{
		m_CS_SPL_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11SamplerState>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::VSSetCB(UINT slot, ComPtr<ID3D11Buffer> pResource)
	{
		m_VS_CB_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11Buffer>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::VSSetSRV(UINT slot, ComPtr<ID3D11ShaderResourceView> pResource)
	{
		m_VS_SRV_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11ShaderResourceView>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::VSSetSPL(UINT slot, ComPtr<ID3D11SamplerState> pResource)
	{
		m_VS_SPL_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11SamplerState>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::PSSetCB(UINT slot, ComPtr<ID3D11Buffer> pResource)
	{
		m_PS_CB_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11Buffer>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::PSSetSRV(UINT slot, ComPtr<ID3D11ShaderResourceView> pResource)
	{
		m_PS_SRV_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11ShaderResourceView>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::PSSetSPL(UINT slot, ComPtr<ID3D11SamplerState> pResource)
	{
		m_PS_SPL_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11SamplerState>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::SetCameraColorOut(std::shared_ptr<RenderTexture> pCameraColor)
	{
		m_pCameraColorOut = pCameraColor;
		return *this;
	}

	Binding& RenderPass::AddBinding(std::shared_ptr<Bindable> pBindable)
	{
		m_bindings.push_back(Binding(std::move(pBindable)));
		return m_bindings[m_bindings.size() - 1];
	}

	Binding& RenderPass::AddBinding(Binding pBinding)
	{
		m_bindings.push_back(std::move(pBinding));
		return m_bindings[m_bindings.size() - 1];
	}

	const RenderTexture& RenderPass::GetCameraColorOut() const
	{
		return *m_pCameraColorOut.get();
	}

	const std::size_t RenderPass::GetHash(std::string passName)
	{
		return std::hash<std::string>{}(std::move(passName));
	}

	RenderPass& RenderPass::GetSubPass(const UINT pass) const
	{
		assert(m_pSubPasses.find(pass) != m_pSubPasses.end() && "Requested RenderPass does not exist!");
		return *m_pSubPasses.at(pass).get();
	}

	const RenderPass& RenderPass::CreateSubPass(const UINT pass)
	{
		assert(m_pSubPasses.find(pass) == m_pSubPasses.end() && "RenderPass cannot be created twice!");
		m_pSubPasses.emplace(pass, std::make_unique<RenderPass>(RenderPassType::Subpass));
		return *m_pSubPasses[pass].get();
	}

	const RenderPass& RenderPass::CreateSubPass(const UINT pass, std::unique_ptr<RenderPass> pRenderPass)
	{
		assert(m_pSubPasses.find(pass) == m_pSubPasses.end() && "RenderPass cannot be created twice!");
		m_pSubPasses.emplace(pass, std::move(pRenderPass));
		return *m_pSubPasses[pass].get();
	}
}
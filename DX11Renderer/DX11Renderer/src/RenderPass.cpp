#include "pch.h"
#include <functional>
#include "RenderPass.h"
#include "Binding.h"
#include "Bindable.h"
#include "RenderTexture.h"
#include "GraphicsDevice.h"

namespace gfx
{

	RenderPass::RenderPass(const RenderPassType renderPassType)
		: m_renderPassType(renderPassType)
	{

	}

	RenderPass::~RenderPass()
	{}

	const RenderPassType RenderPass::GetRenderPassType() const
	{
		return m_renderPassType;
	}

	void RenderPass::EnqueueJob(DrawCommand job)
	{
		m_jobs.push_back(std::move(job));
	}

	void RenderPass::BindSharedResources(const GraphicsDevice& gfx, RenderState& renderState) const
	{
		for (auto& binding : m_bindings)
		{
			binding.Bind(gfx, renderState);
		}

		// todo: can optimize by passing one array for each of these
		for (const auto& bind : m_CS_CB_Binds)
		{
			gfx.GetContext()->CSSetConstantBuffers(bind.first, 1u, bind.second.GetAddressOf());
		}
		for (const auto& bind : m_CS_SRV_Binds)
		{
			gfx.GetContext()->CSSetShaderResources(bind.first, 1u, bind.second.GetAddressOf());
		}
		for (const auto& bind : m_CS_UAV_Binds)
		{
			gfx.GetContext()->CSSetUnorderedAccessViews(bind.first, 1u, bind.second.GetAddressOf(), nullptr);
		}
		for (const auto& bind : m_CS_SPL_Binds)
		{
			gfx.GetContext()->CSSetSamplers(bind.first, 1u, bind.second.GetAddressOf());
		}

		for (const auto& bind : m_VS_CB_Binds)
		{
			gfx.GetContext()->VSSetConstantBuffers(bind.first, 1u, bind.second.GetAddressOf());
		}
		for (const auto& bind : m_VS_SRV_Binds)
		{
			gfx.GetContext()->VSSetShaderResources(bind.first, 1u, bind.second.GetAddressOf());
		}
		for (const auto& bind : m_VS_SPL_Binds)
		{
			gfx.GetContext()->VSSetSamplers(bind.first, 1u, bind.second.GetAddressOf());
		}

		for (const auto& bind : m_DS_CB_Binds)
		{
			gfx.GetContext()->DSSetConstantBuffers(bind.first, 1u, bind.second.GetAddressOf());
		}

		for (const auto& bind : m_PS_CB_Binds)
		{
			gfx.GetContext()->PSSetConstantBuffers(bind.first, 1u, bind.second.GetAddressOf());
		}
		for (const auto& bind : m_PS_SRV_Binds)
		{
			gfx.GetContext()->PSSetShaderResources(bind.first, 1u, bind.second.GetAddressOf());
		}
		for (const auto& bind : m_PS_SPL_Binds)
		{
			gfx.GetContext()->PSSetSamplers(bind.first, 1u, bind.second.GetAddressOf());
		}
	}

	void RenderPass::UnbindSharedResources(const GraphicsDevice& gfx, RenderState& renderState) const
	{
		for (auto& binding : m_bindings)
		{
			binding.Unbind(gfx, renderState);
		}

		if (m_CS_CB_Binds.size() > 0)
		{
			gfx.GetContext()->CSSetConstantBuffers(m_CS_CB_Binds[0].first, m_CS_CB_Binds.size(), RenderConstants::NullBufferArray.data());
		}
		if (m_CS_SRV_Binds.size() > 0)
		{
			gfx.GetContext()->CSSetShaderResources(m_CS_SRV_Binds[0].first, m_CS_SRV_Binds.size(), RenderConstants::NullSRVArray.data());
		}
		if (m_CS_UAV_Binds.size() > 0)
		{
			gfx.GetContext()->CSSetUnorderedAccessViews(m_CS_UAV_Binds[0].first, m_CS_UAV_Binds.size(), RenderConstants::NullUAVArray.data(), nullptr);
		}
		if (m_CS_SPL_Binds.size() > 0)
		{
			gfx.GetContext()->CSSetSamplers(m_CS_SPL_Binds[0].first, m_CS_SPL_Binds.size(), RenderConstants::NullSamplerArray.data());
		}

		if (m_VS_CB_Binds.size() > 0)
		{
			gfx.GetContext()->VSSetConstantBuffers(m_VS_CB_Binds[0].first, m_VS_CB_Binds.size(), RenderConstants::NullBufferArray.data());
		}
		if (m_VS_SRV_Binds.size() > 0)
		{
			gfx.GetContext()->VSSetShaderResources(m_VS_SRV_Binds[0].first, m_VS_SRV_Binds.size(), RenderConstants::NullSRVArray.data());
		}
		if (m_VS_SPL_Binds.size() > 0)
		{
			gfx.GetContext()->VSSetSamplers(m_VS_SPL_Binds[0].first, m_VS_SPL_Binds.size(), RenderConstants::NullSamplerArray.data());
		}

		if (m_DS_CB_Binds.size() > 0)
		{
			gfx.GetContext()->VSSetConstantBuffers(m_DS_CB_Binds[0].first, m_DS_CB_Binds.size(), RenderConstants::NullBufferArray.data());
		}

		if (m_PS_CB_Binds.size() > 0)
		{
			gfx.GetContext()->PSSetConstantBuffers(m_PS_CB_Binds[0].first, m_PS_CB_Binds.size(), RenderConstants::NullBufferArray.data());
		}
		if (m_PS_SRV_Binds.size() > 0)
		{
			gfx.GetContext()->PSSetShaderResources(m_PS_SRV_Binds[0].first, m_PS_SRV_Binds.size(), RenderConstants::NullSRVArray.data());
		}
		if (m_PS_SPL_Binds.size() > 0)
		{
			gfx.GetContext()->PSSetSamplers(m_PS_SPL_Binds[0].first, m_PS_SPL_Binds.size(), RenderConstants::NullSamplerArray.data());
		}

		// todo: check render state here...
		gfx.GetContext()->HSSetShader(nullptr, nullptr, 0u);
		gfx.GetContext()->DSSetShader(nullptr, nullptr, 0u);
	}

	void RenderPass::Execute(const GraphicsDevice& gfx, RenderState& renderState) const
	{
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
		m_DS_CB_Binds.clear();
		m_PS_CB_Binds.clear();
		m_PS_SRV_Binds.clear();
		m_PS_SPL_Binds.clear();
		return *this;
	}

	RenderPass& RenderPass::CSSetCB(const slotUINT slot, const ComPtr<ID3D11Buffer>& pResource)
	{
		m_CS_CB_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11Buffer>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::CSSetSRV(const slotUINT slot, const ComPtr<ID3D11ShaderResourceView>& pResource)
	{
		m_CS_SRV_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11ShaderResourceView>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::CSSetUAV(const slotUINT slot, const ComPtr<ID3D11UnorderedAccessView>& pResource)
	{
		m_CS_UAV_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11UnorderedAccessView>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::CSSetSPL(const slotUINT slot, const ComPtr<ID3D11SamplerState>& pResource)
	{
		m_CS_SPL_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11SamplerState>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::VSSetCB(const slotUINT slot, const ComPtr<ID3D11Buffer>& pResource)
	{
		m_VS_CB_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11Buffer>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::VSSetSRV(const slotUINT slot, const ComPtr<ID3D11ShaderResourceView>& pResource)
	{
		m_VS_SRV_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11ShaderResourceView>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::VSSetSPL(const slotUINT slot, const ComPtr<ID3D11SamplerState>& pResource)
	{
		m_VS_SPL_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11SamplerState>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::DSSetCB(const slotUINT slot, const ComPtr<ID3D11Buffer>& pResource)
	{
		m_DS_CB_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11Buffer>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::PSSetCB(const slotUINT slot, const ComPtr<ID3D11Buffer>& pResource)
	{
		m_PS_CB_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11Buffer>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::PSSetSRV(const slotUINT slot, const ComPtr<ID3D11ShaderResourceView>& pResource)
	{
		m_PS_SRV_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11ShaderResourceView>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::PSSetSPL(const slotUINT slot, const ComPtr<ID3D11SamplerState>& pResource)
	{
		m_PS_SPL_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11SamplerState>>(slot, pResource));
		return *this;
	}

	RenderPass& RenderPass::SetCameraColorOut(std::shared_ptr<RenderTexture> pCameraColor)
	{
		m_pCameraColorOut = std::move(pCameraColor);
		return *this;
	}

	Binding& RenderPass::AddBinding(const std::shared_ptr<Bindable>& pBindable)
	{
		m_bindings.push_back(Binding(pBindable));
		return m_bindings[m_bindings.size() - 1];
	}

	const RenderTexture& RenderPass::GetCameraColorOut() const
	{
		return *m_pCameraColorOut.get();
	}

	const std::size_t RenderPass::GetHash(const std::string passName)
	{
		return std::hash<std::string>{}(passName);
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
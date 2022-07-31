#include "pch.h"
#include "RenderStats.h"

namespace gfx
{

	RenderStats::RenderStats()
	{
	}

	RenderStats::~RenderStats()
	{
	}

	void RenderStats::StartFrame()
	{
		m_meshRenderersVisible = 0u;
		m_meshRenderersCulled = 0u;
		m_lightsVisible = 0u;
		m_lightsCulled = 0u;
		m_shadowsVisible = 0u;
		m_shadowsCulled = 0u;

		m_gpuCallsThisFrame = 0u;
		m_gpuCallsSavedThisFrame = 0u;
	}

	void RenderStats::EndFrame()
	{
	}

	void RenderStats::DrawImguiControlWindow()
	{
		UINT totalRenderers = m_meshRenderersVisible + m_meshRenderersCulled;
		UINT totalLights = m_lightsVisible + m_lightsCulled;
		UINT totalShadows = m_shadowsVisible + m_shadowsCulled;

		if (ImGui::Begin("Render Stats"))
		{
			ImGui::Text("CULLING");
			ImGui::Text((std::string("Renderers visible:   ") + std::to_string(m_meshRenderersVisible) + std::string(" / ") + std::to_string(totalRenderers)).c_str());
			ImGui::Text((std::string("Lights visible:      ") + std::to_string(m_lightsVisible) + std::string(" / ") + std::to_string(totalLights)).c_str());
			ImGui::Text((std::string("Shadowmap passes:    ") + std::to_string(m_shadowsVisible) + std::string(" / ") + std::to_string(totalShadows)).c_str());
			ImGui::Text("COMMANDS");
			ImGui::Text((std::string("GFX commands:        ") + std::to_string(m_gpuCallsThisFrame) + std::string(" / ") + std::to_string(m_gpuCallsThisFrame + m_gpuCallsSavedThisFrame)).c_str());
		}
		ImGui::End();
	}

	void RenderStats::AddVisibleRenderers(const u32 count)
	{
		m_meshRenderersVisible += count;
	}

	void RenderStats::AddCulledRenderers(const u32 count)
	{
		m_meshRenderersCulled += count;
	}

	void RenderStats::AddVisibleLights(const u32 count)
	{
		m_lightsVisible += count;
	}

	void RenderStats::AddCulledLights(const u32 count)
	{
		m_lightsCulled += count;
	}

	void RenderStats::AddVisibleShadows(const u32 count)
	{
		m_shadowsVisible += count;
	}

	void RenderStats::AddCulledShadows(const u32 count)
	{
		m_shadowsCulled += count;
	}

	const u32 RenderStats::GetGPUCallsThisFrame() const
	{
		return m_gpuCallsThisFrame;
	}

	const u32 RenderStats::GetGPUCallsSavedThisFrame() const
	{
		return m_gpuCallsSavedThisFrame;
	}

	void RenderStats::RegisterGPUCall()
	{
		m_gpuCallsThisFrame++;
	}

	void RenderStats::RegisterGPUCalls(const u32 calls)
	{
		m_gpuCallsThisFrame += calls;
	}

	void RenderStats::RegisterGPUCallSaved()
	{
		m_gpuCallsSavedThisFrame++;
	}

	void RenderStats::RegisterGPUCallsSaved(const u32 calls)
	{
		m_gpuCallsSavedThisFrame += calls;
	}

}
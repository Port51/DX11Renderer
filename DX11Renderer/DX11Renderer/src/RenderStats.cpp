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

		m_cpuDrawCallsThisFrame = 0u;
		m_cpuDrawCallsSavedByInstancingThisFrame = 0u;
		m_gpuCommandsThisFrame = 0u;
		m_gpuCallsSavedThisFrame = 0u;
	}

	void RenderStats::EndFrame()
	{
	}

	void RenderStats::DrawImguiControlWindow()
	{
		const u32 totalRenderers = m_meshRenderersVisible + m_meshRenderersCulled;
		const u32 totalLights = m_lightsVisible + m_lightsCulled;
		const u32 totalShadows = m_shadowsVisible + m_shadowsCulled;
		const u32 totalDrawCalls = m_cpuDrawCallsThisFrame + m_cpuDrawCallsSavedByInstancingThisFrame;
		const u32 totalGPUCommands = m_gpuCommandsThisFrame + m_gpuCallsSavedThisFrame;

		if (ImGui::Begin("Render Stats"))
		{
			ImGui::Text("CULLING");
			ImGui::Text((std::string("Renderers visible:     ") + std::to_string(m_meshRenderersVisible) + std::string(" / ") + std::to_string(totalRenderers)).c_str());
			ImGui::Text((std::string("Lights visible:        ") + std::to_string(m_lightsVisible) + std::string(" / ") + std::to_string(totalLights)).c_str());
			ImGui::Text((std::string("Shadowmap passes:      ") + std::to_string(m_shadowsVisible) + std::string(" / ") + std::to_string(totalShadows)).c_str());
			ImGui::Text("");
			ImGui::Text("COMMANDS");
			ImGui::Text((std::string("CPU draw calls:        ") + std::to_string(m_cpuDrawCallsThisFrame)).c_str());
			//ImGui::Text((std::string("Saved via instancing:  ") + std::to_string(m_cpuDrawCallsSavedByInstancingThisFrame)).c_str());
			ImGui::Text((std::string("GPU commands:          ") + std::to_string(m_gpuCommandsThisFrame)).c_str());
			ImGui::Text((std::string("^^ saved by sorting:   ") + std::to_string(m_gpuCallsSavedThisFrame)).c_str());
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

	void RenderStats::RegisterCPUDrawCall()
	{
		m_cpuDrawCallsThisFrame++;
	}

	void RenderStats::RegisterCPUDrawCalls(const u32 calls)
	{
		m_cpuDrawCallsThisFrame += calls;
	}

	void RenderStats::RegisterCPUDrawCallSaved()
	{
		m_cpuDrawCallsSavedByInstancingThisFrame++;
	}

	void RenderStats::RegisterCPUDrawCallsSaved(const u32 calls)
	{
		m_cpuDrawCallsSavedByInstancingThisFrame += calls;
	}

	const u32 RenderStats::GetCPUDrawCallsThisFrame() const
	{
		return m_cpuDrawCallsThisFrame;
	}

	const u32 RenderStats::GetCPUDrawCallsSavedThisFrame() const
	{
		return m_cpuDrawCallsSavedByInstancingThisFrame;
	}

	const u32 RenderStats::GetGPUCallsThisFrame() const
	{
		return m_gpuCommandsThisFrame;
	}

	const u32 RenderStats::GetGPUCallsSavedThisFrame() const
	{
		return m_gpuCallsSavedThisFrame;
	}

	void RenderStats::RegisterGPUCall()
	{
		m_gpuCommandsThisFrame++;
	}

	void RenderStats::RegisterGPUCalls(const u32 calls)
	{
		m_gpuCommandsThisFrame += calls;
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
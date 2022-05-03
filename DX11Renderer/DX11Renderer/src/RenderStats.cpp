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
		}
		ImGui::End();
	}

	void RenderStats::AddVisibleRenderers(const UINT count)
	{
		m_meshRenderersVisible += count;
	}

	void RenderStats::AddCulledRenderers(const UINT count)
	{
		m_meshRenderersCulled += count;
	}

	void RenderStats::AddVisibleLights(const UINT count)
	{
		m_lightsVisible += count;
	}

	void RenderStats::AddCulledLights(const UINT count)
	{
		m_lightsCulled += count;
	}

	void RenderStats::AddVisibleShadows(const UINT count)
	{
		m_shadowsVisible += count;
	}

	void RenderStats::AddCulledShadows(const UINT count)
	{
		m_shadowsCulled += count;
	}

}
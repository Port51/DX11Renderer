#include "pch.h"
#include "RenderStats.h"
#include "Timer.h"
#include <sstream>
#include <iomanip>

namespace gfx
{

	RenderStats::RenderStats()
		: m_pTimer(std::make_unique<Timer>()), m_framesLeftInRecording(framesToRecord)
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
		m_framesLeftInRecording--;
		if (m_framesLeftInRecording <= 0) m_framesLeftInRecording = framesToRecord;
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
			ImGui::Text("");
			ImGui::Text("CPU TIMINGS");

			std::stringstream ss;
			for (const auto& task : m_taskDisplayOrder)
			{
				const float timeInMs = m_taskTimes.at(task).lastAverageTime * 1000.f;

				ss.str(std::string()); // clear sstring
				ss << task;
				for (size_t i = 0; i < 30 - task.size(); ++i) ss << " ";
				ss << ": ";
				if (timeInMs < 10.f) ss << " ";
				ss << std::fixed << std::setprecision(3) << timeInMs << " ms";

				ImGui::Text(ss.str().c_str());
			}
		}
		ImGui::End();
	}

	void RenderStats::StartTaskTimer(const std::string& taskName)
	{
		if (m_taskTimes.find(taskName) == m_taskTimes.end())
		{
			// If new task, add to display order
			m_taskDisplayOrder.emplace_back(taskName);

			TaskTiming newTaskTiming;
			newTaskTiming.startTimeThisFrame = m_pTimer->Peek();
			newTaskTiming.cumulativeTaskTime = 0.f;
			newTaskTiming.lastAverageTime = 0.f;
			m_taskTimes.emplace(taskName, newTaskTiming);
		}
		else
		{
			m_taskTimes[taskName].startTimeThisFrame = m_pTimer->Peek();
		}
	}

	void RenderStats::EndTaskTimer(const std::string& taskName)
	{
		const float duration = m_pTimer->Peek() - m_taskTimes.at(taskName).startTimeThisFrame;
		m_taskTimes[taskName].cumulativeTaskTime += duration;

		if (m_framesLeftInRecording == 1)
		{
			// Save and reset
			m_taskTimes[taskName].lastAverageTime = m_taskTimes.at(taskName).cumulativeTaskTime / static_cast<float>(framesToRecord);
			m_taskTimes[taskName].cumulativeTaskTime = 0.f;
		}
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
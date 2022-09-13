#pragma once
#include "CommonHeader.h"
#include <unordered_map>

namespace gfxcore
{
	class Timer;
}

namespace gfx
{
	class RenderStats
	{
	private:
		static constexpr int framesToRecord = 30;

		struct TaskTiming
		{
		public:
			float startTimeThisFrame;
			float cumulativeTaskTime;
			float lastAverageTime;
		};

	public:
		enum class TimedProcess : u8 { CullAndFrameBindsProcess, ShadowAndLightCull, DrawCallSetupComplete, HZBProcess, GBuffer, SSAO, TiledLighting, ClusteredLighting, Opaque, Skybox, Transparent, SSR, Downsample, };
	public:
		RenderStats();
		~RenderStats();

	public:
		void StartFrame();
		void EndFrame();
		void DrawImguiControlWindow();
		void StartTaskTimer(const std::string& taskName);
		void EndTaskTimer(const std::string& taskName);

	public:
		void AddVisibleRenderers(const u32 count);
		void AddCulledRenderers(const u32 count);
		void AddVisibleLights(const u32 count);
		void AddCulledLights(const u32 count);
		void AddVisibleShadows(const u32 count);
		void AddCulledShadows(const u32 count);

	public:
		void RegisterCPUDrawCall();
		void RegisterCPUDrawCalls(const u32 calls);
		void RegisterCPUDrawCallSaved();
		void RegisterCPUDrawCallsSaved(const u32 calls);
		const u32 GetCPUDrawCallsThisFrame() const;
		const u32 GetCPUDrawCallsSavedThisFrame() const;

		void RegisterGPUCall();
		void RegisterGPUCalls(const u32 calls);
		void RegisterGPUCallSaved();
		void RegisterGPUCallsSaved(const u32 calls);
		const u32 GetGPUCallsThisFrame() const;
		const u32 GetGPUCallsSavedThisFrame() const;

	private:
		u32 m_meshRenderersVisible;
		u32 m_meshRenderersCulled;
		u32 m_lightsVisible;
		u32 m_lightsCulled;
		u32 m_shadowsVisible;
		u32 m_shadowsCulled;

		u32 m_cpuDrawCallsThisFrame;
		u32 m_cpuDrawCallsSavedByInstancingThisFrame;
		u32 m_gpuCommandsThisFrame;
		u32 m_gpuCallsSavedThisFrame;

		std::vector<std::string> m_taskDisplayOrder;
		std::unordered_map<std::string, TaskTiming> m_taskTimes;
		std::unique_ptr<Timer> m_pTimer;
		int m_framesLeftInRecording;

	};
}
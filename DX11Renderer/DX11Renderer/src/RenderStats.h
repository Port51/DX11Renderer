#pragma once
#include "CommonHeader.h"

namespace gfx
{
	class RenderStats
	{
	public:
		RenderStats();
		~RenderStats();
	public:
		void StartFrame();
		void EndFrame();
		void DrawImguiControlWindow();
	public:
		void AddVisibleRenderers(const u32 count);
		void AddCulledRenderers(const u32 count);
		void AddVisibleLights(const u32 count);
		void AddCulledLights(const u32 count);
		void AddVisibleShadows(const u32 count);
		void AddCulledShadows(const u32 count);
	public:
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

		u32 m_gpuCallsThisFrame;
		u32 m_gpuCallsSavedThisFrame;
	};
}
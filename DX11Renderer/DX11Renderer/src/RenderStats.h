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
		void AddVisibleRenderers(const UINT count);
		void AddCulledRenderers(const UINT count);
		void AddVisibleLights(const UINT count);
		void AddCulledLights(const UINT count);
		void AddVisibleShadows(const UINT count);
		void AddCulledShadows(const UINT count);
	private:
		UINT m_meshRenderersVisible;
		UINT m_meshRenderersCulled;
		UINT m_lightsVisible;
		UINT m_lightsCulled;
		UINT m_shadowsVisible;
		UINT m_shadowsCulled;
	};
}
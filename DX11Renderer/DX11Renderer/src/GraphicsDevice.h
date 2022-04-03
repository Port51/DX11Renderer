#pragma once
#include "CommonHeader.h"
#include "WindowsInclude.h"
#include "DXMathInclude.h"
#include <vector>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct IDXGISwapChain;

namespace gfx
{
	class DepthStencilTarget;
	class Log;

	class GraphicsDevice
	{
	public:
		GraphicsDevice(HWND hWnd, int windowWidth, int windowHeight);
		GraphicsDevice(const GraphicsDevice&) = delete; // here because of destructor, but don't want this...
		GraphicsDevice& operator=(const GraphicsDevice&) = delete; // here because of destructor, but don't want this...
		virtual ~GraphicsDevice() = default;
	public:
		void BeginFrame();
		void EndFrame();
		void ClearBuffer(float red, float green, float blue);
		void DrawIndexed(UINT indexCount);
		void DrawIndexedInstanced(UINT indexCount, UINT instanceCount);
		void SetDepthOnlyRenderTarget();
		void SetDepthOnlyRenderTarget(const std::shared_ptr<DepthStencilTarget>& pDepthStencil);
		void SetRenderTarget(ComPtr<ID3D11RenderTargetView> renderTargetView);
		void SetRenderTargets(std::vector<ID3D11RenderTargetView*> renderTargetViews);
		void ClearRenderTargets();
		void SetViewport(int x, int y, int width, int height);
		void SetViewport(int width, int height);
		int GetScreenWidth() const;
		int GetScreenHeight() const;
	public:
		ComPtr<ID3D11Device> GetAdapter() const;
		ComPtr<ID3D11DeviceContext> GetContext() const;
		const std::unique_ptr<Log>& GetLog();
		const std::shared_ptr<DepthStencilTarget>& GetDepthStencilTarget();
		const ComPtr<ID3D11RenderTargetView>& GetBackBufferView();
	public:
		void EnableImgui();
		void DisableImgui();
		bool IsImguiEnabled() const;
	private:
		std::unique_ptr<Log> m_pLog;
		// Allocating stuff
		ComPtr<ID3D11Device> m_pDevice;
		// Used for configuring pipeline and executing render commands
		ComPtr<ID3D11DeviceContext> m_pContext;
		// RT view of backbuffer
		ComPtr<ID3D11RenderTargetView> m_pBackBufferView;
		std::shared_ptr<DepthStencilTarget> m_pDepthStencil;
		ComPtr<IDXGISwapChain> m_pSwapChain;
	private:
		UINT m_currentRenderTargetCount = 0u;
		std::vector<ID3D11RenderTargetView*> m_pNullRenderTargetViews;
	private:
		bool m_imguiEnabled = true;
		int m_screenWidth;
		int m_screenHeight;
	};
}
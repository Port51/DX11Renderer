#include "pch.h"
#include "GraphicsDevice.h"
#include <sstream>
#include <d3dcompiler.h>
#include "DepthStencilTarget.h"
#include <random>
#include "DX11Include.h"
#include "RenderStats.h"
#include "Config.h"
#include "SharedCodex.h"
#include "RandomGenerator.h"
#include <dxgidebug.h>	// For DXGIGetDebugInterface1
#include <dxgi1_3.h>	// For DXGIGetDebugInterface1

namespace gfx
{
	GraphicsDevice::GraphicsDevice(const HWND hWnd, const UINT windowWidth, const UINT windowHeight)
		: m_screenWidth(windowWidth)
		, m_screenHeight(windowHeight)
		, m_pLog(std::make_unique<Log>())
		, m_pRenderStats(std::make_unique<RenderStats>())
	{
		m_pNullRenderTargetViews.resize(10u, nullptr);

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferDesc.Width = 0; // use window size
		swapChainDesc.BufferDesc.Height = 0;
		swapChainDesc.BufferDesc.Format = Config::LinearLighting ? DXGI_FORMAT_B8G8R8A8_UNORM_SRGB : DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0; // use whatever's there already
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = (UINT)Config::MsaaSamples;
		swapChainDesc.SampleDesc.Quality = (UINT)Config::MsaaQuality;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1; // this gives 1 back buffer and 1 front buffer
		swapChainDesc.OutputWindow = hWnd;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // how to present - usually this is the best
		swapChainDesc.Flags = 0;

		UINT swapCreateFlags = 0u;
#ifndef NDEBUG
		swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG; // Will create ID3D11Debug
#endif

		// Create device and front/back buffers, and swap chain and rendering context
		// pass nullptr for defaults
		THROW_IF_FAILED(D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			swapCreateFlags,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&m_pSwapChain,
			&m_pDevice,
			nullptr, // will be filled with feature levels actually secured
			&m_pContext
		));

		// Verify compute shaders are supported
		if (m_pDevice->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0)
		{
			D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS hwopts = { 0 };
			(void)m_pDevice->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &hwopts, sizeof(hwopts));
			if (!hwopts.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
			{
				THROW("DirectCompute is not supported by this device");
			}
		}

		// Gain access to texture subresource in swap chain (back buffer)
		ComPtr<ID3D11Resource> pBackBuffer;
		// Reinterpret here = creating pointer to a pointer
		THROW_IF_FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
		THROW_IF_FAILED(m_pDevice->CreateRenderTargetView(
			pBackBuffer.Get(),
			nullptr,
			&m_pBackBufferView
		));

		m_pDepthStencil = std::make_shared<DepthStencilTarget>(*this, windowWidth, windowHeight);

		//
		// Setup viewport
		//
		SetViewport(windowWidth, windowHeight);

		ImGui_ImplDX11_Init(m_pDevice.Get(), m_pContext.Get());
	}

	GraphicsDevice::~GraphicsDevice()
	{
		Codex::ReleaseAll();

		m_pDepthStencil->Release();
		m_pBackBufferView.Reset();
		m_pSwapChain.Reset();
		m_pContext.Reset();
		m_pDevice.Reset();

		ImGui_ImplDX11_Shutdown();

#if defined(_DEBUG)
		{
			ComPtr<IDXGIDebug1> dxgiDebug;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
			{
				dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
			}
		}
#endif
	}

	void GraphicsDevice::BeginFrame()
	{
		// imgui begin frame
		if (m_imguiEnabled)
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}
	}

	void GraphicsDevice::EndFrame()
	{
		if (m_imguiEnabled)
		{
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		// Sync interval has to do with target framerate
		// If refresh = 60Hz but you are targeting 30FPS, use sync interval of 2u
		HRESULT hr;
		if (FAILED(hr = m_pSwapChain->Present(1u, 0u)))
		{
			// Special handling for this as we need more info
			if (hr == DXGI_ERROR_DEVICE_REMOVED)
			{
				// Could be driver failure, overclocking, etc.
				// Get HRESULT this way
				THROW_ALWAYS(m_pDevice->GetDeviceRemovedReason());
			}
			else
			{
				// Standard exception
				THROW_ALWAYS(hr);
			}
		}
	}

	void GraphicsDevice::ClearBuffer(const float red, const float green, const float blue) const
	{
		const float color[] = { red, green, blue, 1.0f };
		m_pContext->ClearRenderTargetView(m_pBackBufferView.Get(), color);
		m_pContext->ClearDepthStencilView(m_pDepthStencil->GetView().Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);
		REGISTER_GPU_CALLS_GFX((*this), 2u);
	}

	void GraphicsDevice::EnableImgui()
	{
		m_imguiEnabled = true;
	}

	void GraphicsDevice::DisableImgui()
	{
		m_imguiEnabled = false;
	}

	bool GraphicsDevice::IsImguiEnabled() const
	{
		return m_imguiEnabled;
	}

	void GraphicsDevice::DrawIndexed(const UINT indexCount) const
	{
		m_pContext->DrawIndexed(indexCount, 0u, 0u);
		REGISTER_GPU_CALL_GFX((*this));
	}

	void GraphicsDevice::DrawIndexedInstanced(const UINT indexCount, const UINT instanceCount) const
	{
		m_pContext->DrawIndexedInstanced(indexCount, instanceCount, 0u, 0, 0u);
		REGISTER_GPU_CALL_GFX((*this));
	}

	void GraphicsDevice::SetDepthOnlyRenderTarget() const
	{
		m_pContext->OMSetRenderTargets(0u, nullptr, m_pDepthStencil->GetView().Get());
		REGISTER_GPU_CALL_GFX((*this));
	}

	void GraphicsDevice::SetDepthOnlyRenderTarget(const std::shared_ptr<DepthStencilTarget>& _pDepthStencil) const
	{
		m_pContext->OMSetRenderTargets(0u, nullptr, _pDepthStencil->GetView().Get());
		REGISTER_GPU_CALL_GFX((*this));
	}

	void GraphicsDevice::SetRenderTarget(const ComPtr<ID3D11RenderTargetView>& renderTargetView)
	{
		m_pContext->OMSetRenderTargets(1u, renderTargetView.GetAddressOf(), m_pDepthStencil->GetView().Get());
		m_currentRenderTargetCount = 1u;
		REGISTER_GPU_CALL_GFX((*this));
	}

	void GraphicsDevice::SetRenderTargets(const std::vector<ID3D11RenderTargetView*>& renderTargetViews)
	{
		m_pContext->OMSetRenderTargets(renderTargetViews.size(), renderTargetViews.data(), m_pDepthStencil->GetView().Get());
		m_currentRenderTargetCount = renderTargetViews.size();
		REGISTER_GPU_CALL_GFX((*this));
	}

	void GraphicsDevice::ClearRenderTargets() const
	{
		m_pContext->OMSetRenderTargets(m_currentRenderTargetCount, m_pNullRenderTargetViews.data(), nullptr);
		REGISTER_GPU_CALL_GFX((*this));
	}

	void GraphicsDevice::SetViewport(const int x, const int y, const int width, const int height) const
	{
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
		vp.TopLeftX = (FLOAT)x;
		vp.TopLeftY = (FLOAT)y;
		m_pContext->RSSetViewports(1u, &vp);
		REGISTER_GPU_CALL_GFX((*this));
	}

	void GraphicsDevice::SetViewport(const int width, const int height) const
	{
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		m_pContext->RSSetViewports(1u, &vp);
		REGISTER_GPU_CALL_GFX((*this));
	}

	const UINT GraphicsDevice::GetScreenWidth() const
	{
		return m_screenWidth;
	}

	const UINT GraphicsDevice::GetScreenHeight() const
	{
		return m_screenHeight;
	}

	const ComPtr<ID3D11Device>& GraphicsDevice::GetAdapter() const
	{
		return m_pDevice;
	}

	const ComPtr<ID3D11DeviceContext>& GraphicsDevice::GetContext() const
	{
		return m_pContext;
	}

	RenderStats& GraphicsDevice::GetRenderStats() const
	{
		return *m_pRenderStats.get();
	}

	Log& GraphicsDevice::GetLog() const
	{
		return *m_pLog.get();
	}

	const std::shared_ptr<DepthStencilTarget>& GraphicsDevice::GetDepthStencilTarget() const
	{
		return m_pDepthStencil;
	}

	const ComPtr<ID3D11RenderTargetView>& GraphicsDevice::GetBackBufferView() const
	{
		return m_pBackBufferView;
	}
}
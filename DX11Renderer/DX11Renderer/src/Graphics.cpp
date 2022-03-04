#include "pch.h"
#include "Graphics.h"
#include <sstream>
#include <d3dcompiler.h>
#include "DepthStencilTarget.h"
#include <random>
#include "DX11Include.h"
#include "Config.h"

namespace gfx
{
	Graphics::Graphics(HWND hWnd, int windowWidth, int windowHeight)
		: screenWidth(windowWidth)
		, screenHeight(windowHeight)
		, log(std::make_unique<Log>())
	{
		pNullRenderTargetViews.resize(10u, nullptr);

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
		swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG; // Debug will print more to Output window
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
			&pSwapChain,
			&pDevice,
			nullptr, // will be filled with feature levels actually secured
			&pContext
		));

		// Verify compute shader is supported
		if (pDevice->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0)
		{
			D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS hwopts = { 0 };
			(void)pDevice->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &hwopts, sizeof(hwopts));
			if (!hwopts.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
			{
				throw std::runtime_error("DirectCompute is not supported by this device");
			}
		}

		// Gain access to texture subresource in swap chain (back buffer)
		ComPtr<ID3D11Resource> pBackBuffer;
		// Reinterpret here = creating pointer to a pointer
		THROW_IF_FAILED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
		THROW_IF_FAILED(pDevice->CreateRenderTargetView(
			pBackBuffer.Get(),
			nullptr,
			&pBackBufferView
		));

		pDepthStencil = std::make_shared<DepthStencilTarget>(*this, windowWidth, windowHeight);

		//
		// Setup viewport
		//
		SetViewport(windowWidth, windowHeight);

		ImGui_ImplDX11_Init(pDevice.Get(), pContext.Get());
	}

	void Graphics::BeginFrame()
	{
		// imgui begin frame
		if (imguiEnabled)
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}
	}

	void Graphics::EndFrame()
	{
		if (imguiEnabled)
		{
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		// Sync interval has to do with target framerate
		// If refresh = 60Hz but you are targeting 30FPS, use sync interval of 2u
		HRESULT hr;
		if (FAILED(hr = pSwapChain->Present(1u, 0u)))
		{
			// Special handling for this as we need more info
			if (hr == DXGI_ERROR_DEVICE_REMOVED)
			{
				// Could be driver failure, overclocking, etc.
				// Get HRESULT this way
				THROW_ALWAYS(pDevice->GetDeviceRemovedReason());
			}
			else
			{
				// Standard exception
				THROW_ALWAYS(hr);
			}
		}
	}

	void Graphics::ClearBuffer(float red, float green, float blue)
	{
		const float color[] = { red, green, blue, 1.0f };
		pContext->ClearRenderTargetView(pBackBufferView.Get(), color);
		pContext->ClearDepthStencilView(pDepthStencil->GetView().Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);
	}

	void Graphics::EnableImgui()
	{
		imguiEnabled = true;
	}

	void Graphics::DisableImgui()
	{
		imguiEnabled = false;
	}

	bool Graphics::IsImguiEnabled() const
	{
		return imguiEnabled;
	}

	void Graphics::DrawIndexed(UINT count)
	{
		pContext->DrawIndexed(count, 0u, 0u);
	}

	void Graphics::SetDepthOnlyRenderTarget()
	{
		pContext->OMSetRenderTargets(0u, nullptr, pDepthStencil->GetView().Get());
	}

	void Graphics::SetDepthOnlyRenderTarget(const std::shared_ptr<DepthStencilTarget>& _pDepthStencil)
	{
		pContext->OMSetRenderTargets(0u, nullptr, _pDepthStencil->GetView().Get());
	}

	void Graphics::SetRenderTarget(ComPtr<ID3D11RenderTargetView> renderTargetView)
	{
		pContext->OMSetRenderTargets(1u, renderTargetView.GetAddressOf(), pDepthStencil->GetView().Get());
		currentRenderTargetCount = 1u;
	}

	void Graphics::SetRenderTargets(std::vector<ID3D11RenderTargetView*> renderTargetViews)
	{
		pContext->OMSetRenderTargets(renderTargetViews.size(), renderTargetViews.data(), pDepthStencil->GetView().Get());
		currentRenderTargetCount = renderTargetViews.size();
	}

	void Graphics::ClearRenderTargets()
	{
		pContext->OMSetRenderTargets(currentRenderTargetCount, pNullRenderTargetViews.data(), nullptr);
	}

	void Graphics::SetViewport(int x, int y, int width, int height)
	{
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
		vp.TopLeftX = (FLOAT)x;
		vp.TopLeftY = (FLOAT)y;
		pContext->RSSetViewports(1u, &vp);
	}

	void Graphics::SetViewport(int width, int height)
	{
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		pContext->RSSetViewports(1u, &vp);
	}

	int Graphics::GetScreenWidth() const
	{
		return screenWidth;
	}

	int Graphics::GetScreenHeight() const
	{
		return screenHeight;
	}

	ComPtr<ID3D11Device> Graphics::GetDevice() const
	{
		return pDevice;
	}

	ComPtr<ID3D11DeviceContext> Graphics::GetContext() const
	{
		return pContext;
	}

	std::unique_ptr<Log>& Graphics::GetLog()
	{
		return log;
	}
	std::shared_ptr<DepthStencilTarget>& Graphics::GetDepthStencilTarget()
	{
		return pDepthStencil;
	}
	ComPtr<ID3D11RenderTargetView>& Graphics::GetBackBufferView()
	{
		return pBackBufferView;
	}
}
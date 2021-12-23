#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <d3dcompiler.h>
#include "DXMathInclude.h"
#include "GraphicsThrowMacros.h"
#include "DepthStencilTarget.h"
#include "Imgui/imgui_impl_dx11.h"
#include "Imgui/imgui_impl_win32.h"

// Only do this in .cpp files
namespace wrl = Microsoft::WRL;

// Sets up linker settings
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib") // for loading and compiling shaders

Graphics::Graphics(HWND hWnd, int windowWidth, int windowHeight)
	: screenWidth(windowWidth), screenHeight(windowHeight)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc.Width = 0; // use window size
	swapChainDesc.BufferDesc.Height = 0;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0; // use whatever's there already
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1; // basically AA
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1; // this gives 1 back buffer and 1 front buffer
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // how to present - usually this is the best
	swapChainDesc.Flags = 0;

	HRESULT hr;

	UINT swapCreateFlags = 0u;
#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG; // Debug will print more to Output window
#endif

	// Create device and front/back buffers, and swap chain and rendering context
	// pass nullptr for defaults
	GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
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
	wrl::ComPtr<ID3D11Resource> pBackBuffer;
	// Reinterpret here = creating pointer to a pointer
	GFX_THROW_INFO(pSwapChain->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(
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

	HRESULT hr;
#ifndef NDEBUG
	infoManager.Set();
#endif

	// Sync interval has to do with target framerate
	// If refresh = 60Hz but you are targeting 30FPS, use sync interval of 2u
	if (FAILED(hr = pSwapChain->Present(1u, 0u)))
	{
		// Special handling for this as we need more info
		if (hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			// Could be driver failure, overclocking, etc.
			throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
		}
		else
		{
			// Standard exception
			throw GFX_EXCEPT(hr);
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
	GFX_THROW_INFO_ONLY(pContext->DrawIndexed(count, 0u, 0u));
}

void Graphics::SetRenderTarget(Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView)
{
	pContext->OMSetRenderTargets(1u, renderTargetView.GetAddressOf(), pDepthStencil->GetView().Get());
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

// Graphics exception stuff
Graphics::HrException::HrException(int line, const char * file, HRESULT hr, std::vector<std::string> infoMsgs)
	:
	Exception(line, file),
	hr(hr)
{
	// join all info messages with newlines into single string
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	// remove final newline if exists
	if (!info.empty())
	{
		info.pop_back();
	}
}

const char* Graphics::HrException::what() const
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;
	if (!info.empty())
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const
{
	return "Chili Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const
{
	return hr;
}

std::string Graphics::HrException::GetErrorString() const
{
	return DXGetErrorString(hr);
}

std::string Graphics::HrException::GetErrorDescription() const
{
	char buf[512];
	DXGetErrorDescription(hr, buf, sizeof(buf));
	return buf;
}

std::string Graphics::HrException::GetErrorInfo() const
{
	return info;
}

Graphics::InfoException::InfoException(int line, const char * file, std::vector<std::string> infoMsgs)
	:
	Exception(line, file)
{
	// join all info messages with newlines into single string
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	// remove final newline if exists
	if (!info.empty())
	{
		info.pop_back();
	}
}

const char* Graphics::InfoException::what() const
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::InfoException::GetType() const
{
	return "Chili Graphics Info Exception";
}

std::string Graphics::InfoException::GetErrorInfo() const
{
	return info;
}

const char* Graphics::DeviceRemovedException::GetType() const
{
	return "Chili Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}

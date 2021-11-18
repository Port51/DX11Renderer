#pragma once
#include "WindowsInclude.h"
#include "CustomException.h"
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include "DxgiInfoManager.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory>
#include <random>
#include "Log.h"

class Graphics
{
	//friend class Bindable;
public:
	class Exception : public CustomException
	{
		using CustomException::CustomException;
	};
	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {});
		const char* what() const override;
		const char* GetType() const override;
		HRESULT GetErrorCode() const;
		std::string GetErrorString() const;
		std::string GetErrorDescription() const;
		std::string GetErrorInfo() const;
	private:
		HRESULT hr;
		std::string info;
	};
	// For context calls, as those don't return HRESULT
	class InfoException : public Exception
	{
	public:
		InfoException(int line, const char* file, std::vector<std::string> infoMsgs);
		const char* what() const override;
		const char* GetType() const override;
		std::string GetErrorInfo() const;
	private:
		std::string info;
	};
	class DeviceRemovedException : public HrException
	{
		using HrException::HrException;
	public:
		const char* GetType() const override;
	private:
		std::string reason;
	};

public:
	Graphics(HWND hWnd, int windowWidth, int windowHeight);
	Graphics(const Graphics&) = delete; // here because of destructor, but don't want this...
	Graphics& operator=(const Graphics&) = delete; // here because of destructor, but don't want this...
	~Graphics() = default;
	void BeginFrame(float red, float green, float blue);
	void EndFrame();
	void ClearBuffer(float red, float green, float blue);
	void DrawIndexed(UINT count);
	void SetRenderTarget(Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView);
public:
	void EnableImgui();
	void DisableImgui();
	bool IsImguiEnabled() const;
public:
	void SetProjectionMatrix(DirectX::FXMMATRIX proj);
	DirectX::XMMATRIX GetProjectionMatrix() const;
	void SetViewMatrix(DirectX::FXMMATRIX cam);
	DirectX::XMMATRIX GetViewMatrix() const;
public:
	Log log;
#ifndef NDEBUG
	DxgiInfoManager infoManager;
#endif
	// Allocating stuff
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	// Used for configuring pipeline and executing render commands
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
	// RT view of backbuffer
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pBackBufferView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;

private:
	bool imguiEnabled = true;

	// Matrices
	DirectX::XMMATRIX projectionMatrix;
	DirectX::XMMATRIX viewMatrix;

private:

	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain;

};
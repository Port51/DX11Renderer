#pragma once
#include "CommonHeader.h"
#include "WindowsInclude.h"
#include "CustomException.h"
#include "DX11Include.h"
#include "DXMathInclude.h"
#include <vector>
#include "DxgiInfoManager.h"

class DepthStencilTarget;
class Log;

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
public:
	void BeginFrame();
	void EndFrame();
	void ClearBuffer(float red, float green, float blue);
	void DrawIndexed(UINT count);
	void SetRenderTarget(ComPtr<ID3D11RenderTargetView> renderTargetView);
	void SetViewport(int width, int height);
	int GetScreenWidth() const;
	int GetScreenHeight() const;
public:
	ComPtr<ID3D11Device> GetDevice() const;
	ComPtr<ID3D11DeviceContext> GetContext() const;
	std::unique_ptr<Log>& GetLog();
public:
	void EnableImgui();
	void DisableImgui();
	bool IsImguiEnabled() const;
public:
#ifndef NDEBUG
	DxgiInfoManager infoManager;
#endif
	ComPtr<ID3D11RenderTargetView> pBackBufferView; // RT view of backbuffer
	std::shared_ptr<DepthStencilTarget> pDepthStencil;
private:
	std::unique_ptr<Log> log;
	// Allocating stuff
	ComPtr<ID3D11Device> pDevice;
	// Used for configuring pipeline and executing render commands
	ComPtr<ID3D11DeviceContext> pContext;
private:
	bool imguiEnabled = true;
	int screenWidth;
	int screenHeight;

	// Matrices
	dx::XMMATRIX projectionMatrix;
	dx::XMMATRIX viewMatrix;

private:

	ComPtr<IDXGISwapChain> pSwapChain;

};
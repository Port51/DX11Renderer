#pragma once

#include "WindowsInclude.h"
#include "CustomException.h"
#include <d3d11.h>
#include <vector>
#include <wrl.h>
#include "DxgiInfoManager.h"

class Graphics
{
public:
	class Exception : public CustomException
	{
		using CustomException::CustomException;
	};
	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {}) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;
		std::string GetErrorInfo() const noexcept;
	private:
		HRESULT hr;
		std::string info;
	};
	// For context calls, as those don't return HRESULT
	class InfoException : public Exception
	{
	public:
		InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		std::string GetErrorInfo() const noexcept;
	private:
		std::string info;
	};
	class DeviceRemovedException : public HrException
	{
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
	private:
		std::string reason;
	};
public:
	Graphics(HWND hWnd);
	Graphics(const Graphics&) = delete; // here because of destructor, but don't want this...
	Graphics& operator=(const Graphics&) = delete; // here because of destructor, but don't want this...
	~Graphics() = default;
	void EndFrame();
	void ClearBuffer(float red, float green, float blue) noexcept;
	void DrawTestTriangle(float angle);
private:
#ifndef NDEBUG
	DxgiInfoManager infoManager;
#endif
	// Allocating stuff
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
	// Used for configuring pipeline and executing render commands
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
	// RT view of backbuffer
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
};
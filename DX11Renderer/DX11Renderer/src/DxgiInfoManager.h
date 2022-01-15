#pragma once
#include "WindowsInclude.h"
#include "CommonHeader.h"
#include <vector>
#include <string>
#include <dxgidebug.h>

class DxgiInfoManager
{
public:
	DxgiInfoManager();
	~DxgiInfoManager() = default;
	DxgiInfoManager(const DxgiInfoManager&) = delete;
	DxgiInfoManager& operator=(const DxgiInfoManager&) = delete;
	void Set();
	std::vector<std::string> GetMessages() const;
private:
	unsigned long long next = 0u;
	ComPtr<IDXGIInfoQueue> pDxgiInfoQueue;
};
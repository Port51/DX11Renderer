#pragma once

#include "WindowsInclude.h"
#include "ImguiInclude.h"

// DirectX 11
#include "DX11Include.h"
#include "DXMathInclude.h"

// Sets up linker settings
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")		// For ID3D11ShaderReflection (see https://www.gamedev.net/forums/topic/695374-d3dreflect-unrsolved-external-symbol/5372056/)
#pragma comment(lib, "D3DCompiler.lib") // For loading and compiling shaders

// STL Headers
#include <algorithm>
#include <cassert>
#include <chrono>
#include <wrl.h>

#include "Log.h"
#include "WindowsThrowMacros.h"
#include "ExceptionHandling.h"

#define SAFE_RELEASE(p) if (p) (p)->Release()

namespace wrl = Microsoft::WRL;
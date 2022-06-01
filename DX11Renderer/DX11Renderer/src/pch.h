#pragma once

// Windows
#include "WindowsInclude.h"

// Imgui
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
#include <stdlib.h>     // srand, rand

#include "RenderState.h"

#include "Log.h"

// Exceptions
#include "ExceptionHandling.h"

#define SAFE_RELEASE(p) if (p) (p)->Release()

float RandomRange(float min, float max);
dx::XMFLOAT3 RandomColorRGB();
dx::XMFLOAT4 RandomColorRGBA();
dx::XMFLOAT3 RandomSaturatedColorRGB();
dx::XMFLOAT4 RandomSaturatedColorRGBA();

// Return translation portion of TRS matrix
dx::XMFLOAT3 DecomposeMatrixTranslation(const dx::XMFLOAT4X4& trsMatrix);

using namespace Microsoft::WRL;
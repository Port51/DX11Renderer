#pragma once

/*
* GFXCore:
* Shared logic that is used for all Windows graphics projects
*/

#include <wrl.h>
using namespace Microsoft::WRL;

// Support fixed width types
// https://en.cppreference.com/w/cpp/header/cstdint
#include <cstdint>

namespace gfxcore
{
	using i8 = int8_t;
	using i16 = int16_t;
	using i32 = int32_t;
	using i64 = int64_t;
	using u8 = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;
	using f32 = float;
	using f64 = double;
}

#include <memory>
#include <string>

LPCWSTR CharArrayToLPCWSTR(const char* input);

// Helper methods that convert to LPCWSTR or LPCSTR, depending on what's needed
#ifdef UNICODE
#define LPC_STRING_TYPE LPCWSTR
LPCWSTR SafeLPC(const char* input);
LPCWSTR SafeLPC(const std::string& input);
#else
#define LPC_STRING_TYPE LPCSTR
LPCSTR SafeLPC(const char* input);
LPCSTR SafeLPC(const std::string& input);
#endif

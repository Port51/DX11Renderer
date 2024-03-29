#pragma once

#define ZERO_MEM(x) ZeroMemory(&x, sizeof(x))

#include <wrl.h>
using namespace Microsoft::WRL;

// Support fixed width types
// https://en.cppreference.com/w/cpp/header/cstdint
#include <cstdint>

namespace gfx
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

	using guid64 = uint64_t;
	using slotUINT = uint8_t;

	const guid64 NullGuid64 = 0u;
}

#include "TRS.h"
#include <memory>

namespace gfxcore
{ }
using namespace gfxcore;

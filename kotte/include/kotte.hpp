#pragma once
#include <cassert>
#include <string_view>
#include <vector>
#include <raylib.h>
#include <cstdint>

namespace kotte
{
    using u64 = std::uint64_t;
    using i64 = std::int64_t;
    using u32 = std::uint32_t;
    using i32 = std::int32_t;
    using u16 = std::uint16_t;
    using i16 = std::int16_t;
    using u8 = std::uint8_t; //NOTE: std::uint8_t is commonly an alias for 'unsigned char', so streams may treat it as a character.
    using i8 = std::int8_t;
}
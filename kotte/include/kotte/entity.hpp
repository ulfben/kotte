#pragma once

#include <cstdint>
#include <raylib.h>

namespace kotte
{
    enum class EntityKind : std::uint8_t {
        player,
        bomb,
        crate,
        enemy
    };

    struct Entity final {
        EntityKind kind = EntityKind::crate;
        Vector2 world_position{};
    };
}

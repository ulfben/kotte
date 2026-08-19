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

    enum class CardinalDirection : std::uint8_t {
        up,
        right,
        down,
        left
    };

    struct Entity final {
        EntityKind kind = EntityKind::crate;
        Vector2 world_position{};
        CardinalDirection movement_heading = CardinalDirection::right;
    };
}

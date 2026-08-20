#pragma once

#include <compare>
#include <cstdint>
#include <limits>

namespace kotte
{
    struct EntityHandle final {
        static constexpr std::uint32_t invalid_slot = std::numeric_limits<std::uint32_t>::max();

        std::uint32_t slot = invalid_slot;
        std::uint32_t generation = 0;

        [[nodiscard]] auto operator<=>(const EntityHandle&) const noexcept = default;
    };

    // A handle is identity evidence, not proof that an entity is still alive.
    // Only the owning store has enough information to validate it.
}

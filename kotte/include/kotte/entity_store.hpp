#pragma once

#include "kotte/entity.hpp"
#include "kotte/entity_handle.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace kotte
{
    class EntityStore final {
    public:
        void reserve(std::size_t capacity);

        [[nodiscard]] EntityHandle create(Entity entity);
        [[nodiscard]] bool destroy(EntityHandle handle);

        // Resolved pointers are borrowed views and must not cross a structural
        // mutation. Handles remain the durable identity held by other systems.
        [[nodiscard]] Entity* try_get(EntityHandle handle) noexcept;
        [[nodiscard]] const Entity* try_get(EntityHandle handle) const noexcept;
        [[nodiscard]] bool contains(EntityHandle handle) const noexcept;

        [[nodiscard]] std::size_t live_count() const noexcept;
        [[nodiscard]] std::size_t slot_count() const noexcept;

    private:
        static constexpr std::uint32_t first_generation_ = 1;

        struct Slot final {
            explicit Slot(Entity value)
                : entity{std::move(value)}{
            }

            std::optional<Entity> entity;
            std::uint32_t generation = first_generation_;
        };

        [[nodiscard]] static std::uint32_t next_generation(std::uint32_t generation) noexcept;

        // Slots may move when this vector grows, but their numeric positions do
        // not. Handles therefore survive storage reallocation without owning it.
        std::vector<Slot> slots_;
        std::vector<std::uint32_t> free_slots_;
        std::size_t live_count_ = 0;
    };
}

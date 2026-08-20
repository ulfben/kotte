#pragma once

#include "kotte/entity_handle.hpp"

#include <cstddef>
#include <raylib.h>
#include <vector>

namespace kotte
{
    struct CellRange final {
        int first_column = 0;
        int first_row = 0;
        int past_last_column = 0;
        int past_last_row = 0;

        [[nodiscard]] bool operator==(const CellRange&) const noexcept = default;
    };

    struct SpatialQuery final {
        std::vector<EntityHandle> entity_handles;
        std::size_t cells_visited = 0;
        std::size_t candidate_references = 0;
    };

    class SpatialGrid final {
    public:
        SpatialGrid(Vector2 world_size, float cell_size);

        void insert(EntityHandle entity_handle, Rectangle world_bounds);
        void remove(EntityHandle entity_handle, Rectangle world_bounds);
        void update(
            EntityHandle entity_handle,
            Rectangle old_world_bounds,
            Rectangle new_world_bounds);

        [[nodiscard]] SpatialQuery query(Rectangle world_bounds) const;

        [[nodiscard]] int columns() const noexcept;
        [[nodiscard]] int rows() const noexcept;
        [[nodiscard]] float cell_size() const noexcept;

    private:
        using Cell = std::vector<EntityHandle>;

        [[nodiscard]] CellRange cells_overlapping(Rectangle world_bounds) const noexcept;
        [[nodiscard]] Cell& at(int column, int row) noexcept;
        [[nodiscard]] const Cell& at(int column, int row) const noexcept;

        float cell_size_;
        int columns_;
        int rows_;
        std::vector<Cell> cells_;
    };
}

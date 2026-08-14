#include "kotte/spatial_grid.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <stdexcept>

namespace kotte
{
    SpatialGrid::SpatialGrid(Vector2 world_size, float cell_size)
        : cell_size_{cell_size}
        , columns_{0}
        , rows_{0}
    {
        if(world_size.x <= 0.0f || world_size.y <= 0.0f || cell_size <= 0.0f){
            throw std::invalid_argument{"Spatial grid dimensions must be positive."};
        }

        columns_ = static_cast<int>(std::ceil(world_size.x / cell_size_));
        rows_ = static_cast<int>(std::ceil(world_size.y / cell_size_));
        cells_.resize(static_cast<std::size_t>(columns_) * static_cast<std::size_t>(rows_));
    }

    void SpatialGrid::insert(std::size_t entity_index, Rectangle world_bounds){
        const CellRange range = cells_overlapping(world_bounds);

        // An entity can cross a cell boundary, so add its index to every cell
        // touched by its complete world-space bounds.
        for(int row = range.first_row; row < range.past_last_row; ++row){
            for(int column = range.first_column; column < range.past_last_column; ++column){
                Cell& cell = at(column, row);
                assert(std::ranges::find(cell, entity_index) == cell.end());
                cell.push_back(entity_index);
            }
        }
    }

    void SpatialGrid::remove(std::size_t entity_index, Rectangle world_bounds){
        const CellRange range = cells_overlapping(world_bounds);

        // Remove the old references before an entity is inserted at its new
        // location. The entity itself continues to live in Application's vector.
        for(int row = range.first_row; row < range.past_last_row; ++row){
            for(int column = range.first_column; column < range.past_last_column; ++column){
                Cell& cell = at(column, row);
                const std::size_t removed = std::erase(cell, entity_index);
                assert(removed == 1);
                (void) removed;
            }
        }
    }

    void SpatialGrid::update(
        std::size_t entity_index,
        Rectangle old_world_bounds,
        Rectangle new_world_bounds){
        if(cells_overlapping(old_world_bounds) == cells_overlapping(new_world_bounds)){
            return;
        }

        remove(entity_index, old_world_bounds);
        insert(entity_index, new_world_bounds);
    }

    int SpatialGrid::columns() const noexcept{
        return columns_;
    }

    int SpatialGrid::rows() const noexcept{
        return rows_;
    }

    float SpatialGrid::cell_size() const noexcept{
        return cell_size_;
    }

    CellRange SpatialGrid::cells_overlapping(Rectangle world_bounds) const noexcept{
        if(world_bounds.width <= 0.0f || world_bounds.height <= 0.0f){
            return {};
        }

        const int first_column = static_cast<int>(std::floor(world_bounds.x / cell_size_));
        const int first_row = static_cast<int>(std::floor(world_bounds.y / cell_size_));
        const int past_last_column = static_cast<int>(std::ceil((world_bounds.x + world_bounds.width) / cell_size_));
        const int past_last_row = static_cast<int>(std::ceil((world_bounds.y + world_bounds.height) / cell_size_));

        return {
            std::clamp(first_column, 0, columns_),
            std::clamp(first_row, 0, rows_),
            std::clamp(past_last_column, 0, columns_),
            std::clamp(past_last_row, 0, rows_)
        };
    }

    SpatialGrid::Cell& SpatialGrid::at(int column, int row) noexcept{
        assert(column >= 0 && column < columns_ && row >= 0 && row < rows_);
        const std::size_t index = static_cast<std::size_t>(row) * static_cast<std::size_t>(columns_)
            + static_cast<std::size_t>(column);
        return cells_[index];
    }
}

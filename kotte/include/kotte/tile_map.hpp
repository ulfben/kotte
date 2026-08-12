#pragma once

#include "kotte/random.hpp"

#include <cstddef>
#include <cstdint>
#include <raylib.h>
#include <vector>

namespace kotte
{
    enum class TileType : std::uint8_t {
        floor,
        wall
    };

    struct Tile final {
        TileType type = TileType::floor;
        std::uint8_t variation = 0;
    };

    struct TileRange final {
        int first_column = 0;
        int first_row = 0;
        int past_last_column = 0;
        int past_last_row = 0;
    };

    class TileMap final {
    public:
        TileMap(int width, int height);

        [[nodiscard]] int width() const noexcept;
        [[nodiscard]] int height() const noexcept;
        [[nodiscard]] std::size_t tile_count() const noexcept;
        [[nodiscard]] bool contains(int x, int y) const noexcept;

        [[nodiscard]] Tile& at(int x, int y);
        [[nodiscard]] const Tile& at(int x, int y) const;

        [[nodiscard]] TileRange tiles_overlapping(Rectangle world_view, int tile_size) const noexcept;

    private:
        [[nodiscard]] std::size_t index(int x, int y) const;

        int width_;
        int height_;
        std::vector<Tile> tiles_;
    };

    [[nodiscard]] TileMap make_room(int width, int height, Random& random);
}

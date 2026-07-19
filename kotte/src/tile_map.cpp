#include "kotte/tile_map.hpp"

#include <stdexcept>

namespace kotte
{
    TileMap::TileMap(int width, int height)
        : width_{width}
        , height_{height}
    {
        if(width <= 0 || height <= 0){
            throw std::invalid_argument{"Tile map dimensions must be positive."};
        }

        tiles_.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
    }

    int TileMap::width() const noexcept{
        return width_;
    }

    int TileMap::height() const noexcept{
        return height_;
    }

    std::size_t TileMap::tile_count() const noexcept{
        return tiles_.size();
    }

    bool TileMap::contains(int x, int y) const noexcept{
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    Tile& TileMap::at(int x, int y){
        return tiles_.at(index(x, y));
    }

    const Tile& TileMap::at(int x, int y) const{
        return tiles_.at(index(x, y));
    }

    std::size_t TileMap::index(int x, int y) const{
        if(!contains(x, y)){
            throw std::out_of_range{"Tile coordinates are outside the map."};
        }

        return static_cast<std::size_t>(y) * static_cast<std::size_t>(width_)
            + static_cast<std::size_t>(x);
    }

    TileMap make_room(int width, int height, Random& random){
        TileMap room{width, height};

        for(int y = 0; y < room.height(); ++y){
            for(int x = 0; x < room.width(); ++x){
                Tile& tile = room.at(x, y);
                const bool boundary = x == 0 || y == 0
                    || x == room.width() - 1 || y == room.height() - 1;

                tile.type = boundary ? TileType::wall : TileType::floor;
                tile.variation = random.next<3, std::uint8_t>(); //0-2, returned as an uint8_t
            }
        }

        return room;
    }
}

#pragma once

#include "kotte/random.hpp"
#include "kotte/tile_map.hpp"
#include "kotte/window.hpp"

#include <cstdint>
#include <raylib.h>
#include <string_view>

namespace kotte
{
    class Application final{
    public:
        Application(
            int window_width,
            int window_height,
            std::string_view title,
            std::uint64_t seed,
            int target_fps = 0);

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        void run();
        void request_exit() noexcept;

    private:
        void update(float delta_time);
        void render() const;
        void try_move_player(int delta_x, int delta_y);
        static constexpr int tile_size_ = 40;
        static constexpr Color background_color_{0x11, 0x22, 0x33, 0xff};
        static constexpr Color floor_colors_[3]{
            Color{0x38, 0x49, 0x52, 0xff},
            Color{0x3d, 0x4f, 0x59, 0xff},
            Color{0x42, 0x55, 0x60, 0xff}
        };
        static constexpr Color player_color_{0xf2, 0xc1, 0x4e, 0xff};
        static constexpr Color wall_color_{0x78, 0x5f, 0x47, 0xff};

        Window window_; // Constructed first, destroyed last.
        Random random_;
        TileMap map_;
        std::uint64_t seed_;
        int player_x_ = 2;
        int player_y_ = 2;
        bool exit_requested_ = false;
    };
}

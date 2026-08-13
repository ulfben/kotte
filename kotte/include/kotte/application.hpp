#pragma once

#include "kotte/camera.hpp"
#include "kotte/entity.hpp"
#include "kotte/random.hpp"
#include "kotte/tile_map.hpp"
#include "kotte/window.hpp"

#include <cstddef>
#include <cstdint>
#include <raylib.h>
#include <string_view>
#include <vector>

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
        void update_player(float delta_time) noexcept;
        void update_camera(float delta_time) noexcept;
        void render() const;
        void populate_entities();
        [[nodiscard]] Entity& player() noexcept;
        [[nodiscard]] const Entity& player() const noexcept;
        [[nodiscard]] Rectangle entity_world_bounds(const Entity& entity) const noexcept;
        [[nodiscard]] static Color entity_color(EntityKind kind) noexcept;
        [[nodiscard]] static float entity_roundness(EntityKind kind) noexcept;
        static constexpr int tile_size_ = 40;
        static constexpr float player_speed_ = 240.0f;
        static constexpr Color background_color_{0x11, 0x22, 0x33, 0xff};
        static constexpr Color floor_colors_[3]{
            Color{0x38, 0x49, 0x52, 0xff},
            Color{0x3d, 0x4f, 0x59, 0xff},
            Color{0x42, 0x55, 0x60, 0xff}
        };
        static constexpr Color player_color_{0xf2, 0xc1, 0x4e, 0xff};
        static constexpr Color bomb_color_{0x20, 0x24, 0x2a, 0xff};
        static constexpr Color crate_color_{0xb0, 0x72, 0x3c, 0xff};
        static constexpr Color enemy_color_{0xd9, 0x4f, 0x70, 0xff};
        static constexpr Color wall_color_{0x78, 0x5f, 0x47, 0xff};

        Window window_; // Constructed first, destroyed last.
        Camera camera_;
        Random random_;
        TileMap map_;
        std::vector<Entity> entities_;
        std::uint64_t seed_;
        std::size_t player_index_ = 0;
        bool exit_requested_ = false;
    };
}

#pragma once

#include "kotte/camera.hpp"
#include "kotte/entity.hpp"
#include "kotte/random.hpp"
#include "kotte/renderer.hpp"
#include "kotte/spatial_grid.hpp"
#include "kotte/tile_map.hpp"
#include "kotte/window.hpp"

#include <cstddef>
#include <cstdint>
#include <raylib.h>
#include <string_view>
#include <vector>

namespace kotte
{
    struct CollisionDiagnostics final {
        std::size_t enemy_updates = 0;
        std::size_t movement_attempts = 0;
        std::size_t spatial_queries = 0;
        std::size_t cells_visited = 0;
        std::size_t candidate_references = 0;
        std::size_t unique_candidates = 0;
        std::size_t exact_tests = 0;
        std::size_t contacts = 0;
        std::size_t boundary_blocks = 0;
        std::size_t blocked_moves = 0;
        std::size_t enemy_turns = 0;
    };

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
        struct FrameActions final {
            Vector2 player_movement_direction{};
        };

        void run_frame(float delta_time);
        void begin_frame() noexcept;
        void collect_frame_actions() noexcept;
        void update_movers(float delta_time);
        void update_timed_gameplay(float delta_time) noexcept;
        void resolve_gameplay_facts() noexcept;
        void apply_structural_mutations() noexcept;
        void update_presentation(float delta_time) noexcept;
        void update_player(float delta_time);
        void try_move_player(Vector2 axis_displacement);
        void update_enemies(float delta_time);
        void update_camera(float delta_time) noexcept;
        void render();
        void populate_entities();
        void initialize_enemies();
        void populate_spatial_grid();
        [[nodiscard]] Entity& player() noexcept;
        [[nodiscard]] const Entity& player() const noexcept;
        [[nodiscard]] Rectangle entity_world_bounds(const Entity& entity) const noexcept;
        [[nodiscard]] Rectangle entity_collision_bounds(const Entity& entity) const noexcept;
        [[nodiscard]] bool entity_movement_is_blocked(
            std::size_t moving_entity_index,
            Vector2 proposed_position);
        [[nodiscard]] static bool is_solid(EntityKind kind) noexcept;
        [[nodiscard]] static Vector2 cardinal_vector(CardinalDirection direction) noexcept;
        [[nodiscard]] CardinalDirection different_enemy_direction(CardinalDirection current_direction);
        [[nodiscard]] static Color entity_color(EntityKind kind) noexcept;
        [[nodiscard]] static float entity_roundness(EntityKind kind) noexcept;
        static constexpr int tile_size_ = 40;
        static constexpr int spatial_cell_tiles_ = 8;
        static constexpr float spatial_cell_size_ = tile_size_ * spatial_cell_tiles_;
        static constexpr float player_speed_ = 240.0f;
        static constexpr float enemy_speed_ = 80.0f;
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
        std::vector<std::size_t> enemy_indices_;
        SpatialGrid spatial_grid_;
        Renderer renderer_;
        FrameActions frame_actions_;
        CollisionDiagnostics collision_diagnostics_;
        std::uint64_t seed_;
        std::size_t player_index_ = 0;
        bool exit_requested_ = false;
    };
}

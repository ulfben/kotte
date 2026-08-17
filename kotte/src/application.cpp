#include "kotte/application.hpp"

#include <cmath>
#include <format>
#include <raymath.h>
#include <string>

namespace kotte
{    
    Application::Application(
        int width,
        int height,
        std::string_view title,
        std::uint64_t seed,
        int target_fps)
        : window_{width, height, title, target_fps}
        , camera_{window_.size()}
        , random_{seed}
        , map_{make_room(200, 120, random_)}
        , spatial_grid_{{
            static_cast<float>(map_.width() * tile_size_),
            static_cast<float>(map_.height() * tile_size_)}, spatial_cell_size_}
        , seed_{seed}{
        populate_entities();
        initialize_enemies();
        populate_spatial_grid();
    }

    void Application::run(){
        while(!window_.should_close() && !exit_requested_){
            update(GetFrameTime());            
            render();
        }
    }

    void Application::update(float delta_time){
        collision_diagnostics_ = {};

        if(IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_Q)){
            request_exit();
        }

        update_player(delta_time);
        update_enemies(delta_time);
        update_camera(delta_time);
    }

    void Application::update_player(float delta_time){
        Vector2 direction{};
        if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)){
            direction.x -= 1.0f;
        }
        if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)){
            direction.x += 1.0f;
        }
        if(IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)){
            direction.y -= 1.0f;
        }
        if(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)){
            direction.y += 1.0f;
        }

        // Two held keys produce a longer diagonal vector. Normalize it so the
        // player's speed is the same in every direction before scaling it.
        if(direction.x != 0.0f || direction.y != 0.0f){
            direction = Vector2Normalize(direction);
        }

        Entity& player_entity = player();
        const Rectangle old_world_bounds = entity_world_bounds(player_entity);
        const Vector2 displacement = direction * (player_speed_ * delta_time);

        // Treat the two axes as separate proposed movements. Week 5 will use
        // this split to let a clear axis slide along a blocked one.
        if(displacement.x != 0.0f){
            try_move_player({displacement.x, 0.0f});
        }
        if(displacement.y != 0.0f){
            try_move_player({0.0f, displacement.y});
        }

        const Rectangle player_bounds = entity_world_bounds(player_entity);
        const Vector2 bounds_offset = player_entity.world_position - Vector2{player_bounds.x, player_bounds.y};
        const float floor_left = static_cast<float>(tile_size_);
        const float floor_top = static_cast<float>(tile_size_);
        const float floor_right = static_cast<float>((map_.width() - 1) * tile_size_);
        const float floor_bottom = static_cast<float>((map_.height() - 1) * tile_size_);
        const Vector2 minimum_position{floor_left + bounds_offset.x, floor_top + bounds_offset.y};
        const Vector2 maximum_position{floor_right - bounds_offset.x, floor_bottom};
        player_entity.world_position = Vector2Clamp(player_entity.world_position, minimum_position, maximum_position);

        const Rectangle new_world_bounds = entity_world_bounds(player_entity);
        spatial_grid_.update(player_index_, old_world_bounds, new_world_bounds);
    }

    void Application::try_move_player(Vector2 axis_displacement){
        ++collision_diagnostics_.movement_attempts;

        Entity& player_entity = player();
        const Vector2 proposed_position = player_entity.world_position + axis_displacement;
        if(entity_movement_is_blocked(player_index_, proposed_position)){
            ++collision_diagnostics_.blocked_moves;
            return;
        }

        player_entity.world_position = proposed_position;
    }

    void Application::update_enemies(float delta_time){
        // The update list contains every enemy, not only those returned by the
        // camera query. Off-screen enemies therefore keep simulating normally.
        for(const std::size_t enemy_index : enemy_indices_){
            ++collision_diagnostics_.enemy_updates;
            ++collision_diagnostics_.movement_attempts;

            Entity& enemy = entities_[enemy_index];
            const Rectangle old_world_bounds = entity_world_bounds(enemy);
            const Vector2 direction = cardinal_vector(enemy.movement_heading);
            const Vector2 displacement = direction * (enemy_speed_ * delta_time);
            const Vector2 proposed_position = enemy.world_position + displacement;

            if(entity_movement_is_blocked(enemy_index, proposed_position)){
                // A blocked enemy stays where it is and chooses a genuinely
                // different heading. It waits until the next update to retry.
                ++collision_diagnostics_.blocked_moves;
                ++collision_diagnostics_.enemy_turns;
                enemy.movement_heading = different_enemy_direction(enemy.movement_heading);
                continue;
            }

            enemy.world_position = proposed_position;
            const Rectangle new_world_bounds = entity_world_bounds(enemy);

            // Enemies are also blockers. Synchronize this accepted move now so
            // the next enemy sees the current position rather than stale cells.
            spatial_grid_.update(enemy_index, old_world_bounds, new_world_bounds);
        }
    }

    void Application::update_camera(float delta_time) noexcept{
        (void) delta_time; // name the argument to avoid the static analyzer yelling at us. :) We'll animate the camera later.
        camera_.set_viewport_size(window_.size());
        camera_.set_centre(player().world_position);
    }

    void Application::render(){
        Frame frame{background_color_};

        const Rectangle world_view = camera_.world_view();
        const TileRange visible_tiles = map_.tiles_overlapping(world_view, tile_size_);
        std::size_t tiles_rendered = 0;

        for(int y = visible_tiles.first_row; y < visible_tiles.past_last_row; ++y){
            for(int x = visible_tiles.first_column; x < visible_tiles.past_last_column; ++x){
                const Tile& tile = map_.at(x, y);
                const Color color = tile.type == TileType::wall
                    ? wall_color_
                    : floor_colors_[tile.variation % 3];

                const Vector2 tile_world_position{
                    static_cast<float>(x * tile_size_),
                    static_cast<float>(y * tile_size_)
                };
                const Vector2 tile_screen_position = camera_.world_to_screen(tile_world_position);

                DrawRectangle(
                    static_cast<int>(std::floor(tile_screen_position.x)),
                    static_cast<int>(std::floor(tile_screen_position.y)),
                    tile_size_ - 1,
                    tile_size_ - 1,
                    color);
                ++tiles_rendered;
            }
        }

        const SpatialQuery spatial_query = spatial_grid_.query(world_view);
        std::size_t exact_visibility_tests = 0;
        std::size_t visible_entities = 0;
        renderer_.clear();

        // The spatial query replaces Week 3's complete entity scan. Its result is
        // only a candidate list, so every candidate still needs an exact test.
        for(const std::size_t entity_index : spatial_query.entity_indices){
            const Entity& entity = entities_[entity_index];
            Rectangle bounds = entity_world_bounds(entity);
            ++exact_visibility_tests;
            if(!CheckCollisionRecs(bounds, world_view)){
                continue;
            }

            ++visible_entities;
            const Vector2 screen_position = camera_.world_to_screen({bounds.x, bounds.y});
            bounds.x = screen_position.x;
            bounds.y = screen_position.y;
            const RenderLayer layer = entity.kind == EntityKind::bomb ? RenderLayer::ground : RenderLayer::world;
            renderer_.submit({bounds, entity_color(entity.kind), layer, entity.world_position.y, entity_roundness(entity.kind)});
        }
        renderer_.sort();
        renderer_.execute();

        std::string diagnostics;

        DrawFPS(GetScreenWidth() - 100, 2);
        diagnostics = std::format("seed: {}", seed_);
        DrawText(diagnostics.c_str(), 10, 10, 20, RAYWHITE);

        diagnostics = std::format(
            "world: {} x {} tiles | {} total", map_.width(), map_.height(), map_.tile_count());
        DrawText(diagnostics.c_str(), 10, 34, 20, RAYWHITE);

        diagnostics = std::format(
            "view: ({:.0f}, {:.0f}) {:.0f} x {:.0f} world pixels", world_view.x, world_view.y, world_view.width, world_view.height);
        DrawText(diagnostics.c_str(), 10, 58, 20, RAYWHITE);

        diagnostics = std::format(
            "visible range: {} x {} tiles", visible_tiles.past_last_column - visible_tiles.first_column, visible_tiles.past_last_row - visible_tiles.first_row);
        DrawText(diagnostics.c_str(), 10, 82, 20, RAYWHITE);

        const double percent_of_world = map_.tile_count() == 0 ? 0.0 : 100.0 * static_cast<double>(tiles_rendered) / static_cast<double>(map_.tile_count());
        diagnostics = std::format(
            "tiles rendered: {} ({:.1f}% of world)", tiles_rendered, percent_of_world);
        DrawText(diagnostics.c_str(), 10, 106, 20, RAYWHITE);

        diagnostics = std::format(
            "range: columns [{}, {}) | rows [{}, {})", visible_tiles.first_column, visible_tiles.past_last_column, visible_tiles.first_row, visible_tiles.past_last_row);
        DrawText(diagnostics.c_str(), 10, 130, 20, RAYWHITE);

        diagnostics = std::format(
            "spatial: {} x {} cells @ {:.0f} px | {} visited | {} refs",
            spatial_grid_.columns(), spatial_grid_.rows(), spatial_grid_.cell_size(),
            spatial_query.cells_visited, spatial_query.candidate_references);
        DrawText(diagnostics.c_str(), 10, 154, 20, RAYWHITE);

        diagnostics = std::format(
            "entities: {} total | {} exact tests | {} visible | {} commands",
            entities_.size(), exact_visibility_tests, visible_entities, renderer_.command_count());
        DrawText(diagnostics.c_str(), 10, 178, 20, RAYWHITE);

        diagnostics = std::format(
            "movement: {} enemies | {} attempts | {} blocked | {} turns",
            collision_diagnostics_.enemy_updates,
            collision_diagnostics_.movement_attempts,
            collision_diagnostics_.blocked_moves,
            collision_diagnostics_.enemy_turns);
        DrawText(diagnostics.c_str(), 10, 202, 20, RAYWHITE);

        diagnostics = std::format(
            "collision: {} queries | {} cells | {} refs | {} candidates",
            collision_diagnostics_.spatial_queries,
            collision_diagnostics_.cells_visited,
            collision_diagnostics_.candidate_references,
            collision_diagnostics_.unique_candidates);
        DrawText(diagnostics.c_str(), 10, 226, 20, RAYWHITE);

        diagnostics = std::format(
            "narrow: {} exact tests | {} contacts | {} boundary blocks",
            collision_diagnostics_.exact_tests,
            collision_diagnostics_.contacts,
            collision_diagnostics_.boundary_blocks);
        DrawText(diagnostics.c_str(), 10, 250, 20, RAYWHITE);

        DrawText("move: WASD/arrows | quit: Q/Escape", 10, window_.height() - 30, 20, LIGHTGRAY);
    }

    void Application::populate_entities(){
        entities_.reserve(map_.tile_count() / 4); // pre-allocation is a fantastic performance win. We'll have ~25% of placing an object on a tile, so tile_count/4 is a good first estimate for the capacity needed.
        entities_.push_back({EntityKind::player, {tile_size_ * 2.5f, tile_size_ * 2.5f}});

        for(int y = 1; y < map_.height() - 1; ++y){
            for(int x = 1; x < map_.width() - 1; ++x){
                if((x == 2 && y == 2) || map_.at(x, y).type != TileType::floor || !random_.coin_flip(0.25f)){
                    continue;
                }

                const std::uint8_t kind_roll = random_.next<20, std::uint8_t>();
                const EntityKind kind = kind_roll < 15
                    ? EntityKind::crate
                    : kind_roll < 19 ? EntityKind::bomb : EntityKind::enemy;
                const Vector2 world_position{
                    static_cast<float>(x * tile_size_ + tile_size_ / 2),
                    static_cast<float>(y * tile_size_ + tile_size_ / 2)
                };
                entities_.push_back({kind, world_position});
            }
        }
    }

    void Application::initialize_enemies(){
        // Finish every world-generation roll before drawing runtime headings.
        // This preserves the complete Week 4 layout for the reference seed.
        for(std::size_t index = 0; index < entities_.size(); ++index){
            Entity& entity = entities_[index];
            if(entity.kind != EntityKind::enemy){
                continue;
            }

            enemy_indices_.push_back(index);
            entity.movement_heading = static_cast<CardinalDirection>(
                random_.next<4, std::uint8_t>());
        }
    }

    void Application::populate_spatial_grid(){
        // Entity construction is complete before the grid stores indices. Week 4
        // keeps this vector fixed, so those non-owning indices remain valid.
        for(std::size_t index = 0; index < entities_.size(); ++index){
            spatial_grid_.insert(index, entity_world_bounds(entities_[index]));
        }
    }

    Entity& Application::player() noexcept{
        return entities_[player_index_];
    }

    const Entity& Application::player() const noexcept{
        return entities_[player_index_];
    }

    Rectangle Application::entity_world_bounds(const Entity& entity) const noexcept{
        Vector2 size{};
        switch(entity.kind){
        case EntityKind::player: size = {24.0f, 32.0f}; break;
        case EntityKind::bomb: size = {22.0f, 22.0f}; break;
        case EntityKind::crate: size = {36.0f, 48.0f}; break;
        case EntityKind::enemy: size = {28.0f, 30.0f}; break;
        }

        const Vector2 top_left = entity.world_position - Vector2{size.x / 2.0f, size.y};
        return {top_left.x, top_left.y, size.x, size.y};
    }

    Rectangle Application::entity_collision_bounds(const Entity& entity) const noexcept{
        // Collision uses a smaller footprint around the part touching the
        // floor. The complete visual rectangle remains in the spatial grid so
        // broad-phase queries conservatively find every possible contact.
        Vector2 size{};
        switch(entity.kind){
        case EntityKind::player: size = {20.0f, 16.0f}; break;
        case EntityKind::bomb: size = {18.0f, 12.0f}; break;
        case EntityKind::crate: size = {32.0f, 32.0f}; break;
        case EntityKind::enemy: size = {24.0f, 14.0f}; break;
        }

        const Vector2 top_left = entity.world_position - Vector2{size.x / 2.0f, size.y};
        return {top_left.x, top_left.y, size.x, size.y};
    }

    bool Application::entity_movement_is_blocked(
        std::size_t moving_entity_index,
        Vector2 proposed_position){
        Entity proposed_entity = entities_[moving_entity_index];
        proposed_entity.world_position = proposed_position;

        // Room walls are a simple boundary rule in the current map. Test the
        // complete visual bounds directly before asking the grid about entities.
        const Rectangle proposed_world_bounds = entity_world_bounds(proposed_entity);
        const float floor_left = static_cast<float>(tile_size_);
        const float floor_top = static_cast<float>(tile_size_);
        const float floor_right = static_cast<float>((map_.width() - 1) * tile_size_);
        const float floor_bottom = static_cast<float>((map_.height() - 1) * tile_size_);
        const bool leaves_room = proposed_world_bounds.x < floor_left
            || proposed_world_bounds.y < floor_top
            || proposed_world_bounds.x + proposed_world_bounds.width > floor_right
            || proposed_world_bounds.y + proposed_world_bounds.height > floor_bottom;
        if(leaves_room){
            ++collision_diagnostics_.boundary_blocks;
            return true;
        }

        const Rectangle proposed_collision_bounds = entity_collision_bounds(proposed_entity);
        const SpatialQuery spatial_query = spatial_grid_.query(proposed_collision_bounds);
        ++collision_diagnostics_.spatial_queries;
        collision_diagnostics_.cells_visited += spatial_query.cells_visited;
        collision_diagnostics_.candidate_references += spatial_query.candidate_references;
        collision_diagnostics_.unique_candidates += spatial_query.entity_indices.size();

        bool blocked = false;

        // A spatial query returns possible contacts. Remove the mover and
        // non-solid kinds before exact-testing the smaller gameplay footprints.
        for(const std::size_t candidate_index : spatial_query.entity_indices){
            if(candidate_index == moving_entity_index){
                continue;
            }

            const Entity& candidate = entities_[candidate_index];
            if(!is_solid(candidate.kind)){
                continue;
            }

            ++collision_diagnostics_.exact_tests;
            if(CheckCollisionRecs(proposed_collision_bounds, entity_collision_bounds(candidate))){
                ++collision_diagnostics_.contacts;
                blocked = true;
            }
        }

        return blocked;
    }

    bool Application::is_solid(EntityKind kind) noexcept{
        switch(kind){
        case EntityKind::player:
        case EntityKind::crate:
        case EntityKind::enemy:
            return true;
        case EntityKind::bomb:
            return false;
        }

        return false;
    }

    Vector2 Application::cardinal_vector(CardinalDirection direction) noexcept{
        switch(direction){
        case CardinalDirection::up: return {0.0f, -1.0f};
        case CardinalDirection::right: return {1.0f, 0.0f};
        case CardinalDirection::down: return {0.0f, 1.0f};
        case CardinalDirection::left: return {-1.0f, 0.0f};
        }

        return {};
    }

    CardinalDirection Application::different_enemy_direction(CardinalDirection current_direction){
        constexpr std::uint8_t direction_count = 4;
        const std::uint8_t current_value = static_cast<std::uint8_t>(current_direction);

        // Adding 1, 2, or 3 modulo four reaches every heading except the one
        // that was just blocked.
        const std::uint8_t offset = static_cast<std::uint8_t>(
            random_.next<3, std::uint8_t>() + 1);
        const std::uint8_t new_value = static_cast<std::uint8_t>(
            (current_value + offset) % direction_count);
        return static_cast<CardinalDirection>(new_value);
    }

    Color Application::entity_color(EntityKind kind) noexcept{
        switch(kind){
        case EntityKind::player: return player_color_;
        case EntityKind::bomb: return bomb_color_;
        case EntityKind::crate: return crate_color_;
        case EntityKind::enemy: return enemy_color_;
        }
        return MAGENTA;
    }

    float Application::entity_roundness(EntityKind kind) noexcept{
        return kind == EntityKind::crate ? 0.15f : 0.8f;
    }

    void Application::request_exit() noexcept{
        exit_requested_ = true;
    }

} //kotte

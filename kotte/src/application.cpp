#include "kotte/application.hpp"

#include <algorithm>
#include <cmath>
#include <format>
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
        , seed_{seed}{
        populate_entities();
    }

    void Application::run(){
        while(!window_.should_close() && !exit_requested_){
            update(GetFrameTime());            
            render();
        }
    }

    void Application::update(float delta_time){
        if(IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_Q)){
            request_exit();
        }

        update_player(delta_time);
        update_camera(delta_time);
    }

    void Application::update_player(float delta_time) noexcept{
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

        const float length_squared = direction.x * direction.x + direction.y * direction.y;
        if(length_squared > 0.0f){
            const float inverse_length = 1.0f / std::sqrt(length_squared);
            direction.x *= inverse_length;
            direction.y *= inverse_length;
        }

        Entity& player_entity = player();
        player_entity.world_position.x += direction.x * player_speed_ * delta_time;
        player_entity.world_position.y += direction.y * player_speed_ * delta_time;

        const Rectangle player_bounds = entity_world_bounds(player_entity);
        const float left_offset = player_entity.world_position.x - player_bounds.x;
        const float top_offset = player_entity.world_position.y - player_bounds.y;
        const float floor_left = static_cast<float>(tile_size_);
        const float floor_top = static_cast<float>(tile_size_);
        const float floor_right = static_cast<float>((map_.width() - 1) * tile_size_);
        const float floor_bottom = static_cast<float>((map_.height() - 1) * tile_size_);
        player_entity.world_position.x = std::clamp(player_entity.world_position.x, floor_left + left_offset, floor_right - left_offset);
        player_entity.world_position.y = std::clamp(player_entity.world_position.y, floor_top + top_offset, floor_bottom);
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

        std::size_t entities_tested = 0;
        std::size_t visible_entities = 0;
        renderer_.clear();
        for(const Entity& entity : entities_){
            Rectangle bounds = entity_world_bounds(entity);
            ++entities_tested;
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
            "entities: {} total | {} tested | {} visible | {} commands",
            entities_.size(), entities_tested, visible_entities, renderer_.command_count());
        DrawText(diagnostics.c_str(), 10, 154, 20, RAYWHITE);
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

        return {
            entity.world_position.x - size.x / 2.0f,
            entity.world_position.y - size.y,
            size.x,
            size.y
        };
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

#include "kotte/application.hpp"

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

        if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)){
            try_move_player(-1, 0);
        }
        if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)){
            try_move_player(1, 0);
        }
        if(IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)){
            try_move_player(0, -1);
        }
        if(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)){
            try_move_player(0, 1);
        }

        update_camera(delta_time);
    }

    void Application::update_camera(float delta_time) noexcept{
        (void) delta_time; // name the argument to avoid the static analyzer yelling at us. :) We'll animate the camera later.
        camera_.set_viewport_size(window_.size());
        camera_.set_centre(player().world_position);
    }

    void Application::render() const{
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
                    static_cast<int>(tile_screen_position.x),
                    static_cast<int>(tile_screen_position.y),
                    tile_size_ - 1,
                    tile_size_ - 1,
                    color);
                ++tiles_rendered;
            }
        }

        std::size_t entity_draw_submissions = 0;
        for(const Entity& entity : entities_){
            Rectangle bounds = entity_world_bounds(entity);
            const Vector2 screen_position = camera_.world_to_screen({bounds.x, bounds.y});
            bounds.x = screen_position.x;
            bounds.y = screen_position.y;
            DrawRectangleRounded(bounds, entity_roundness(entity.kind), 6, entity_color(entity.kind));
            ++entity_draw_submissions;
        }

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
            "entities: {} total | {} draw submissions", entities_.size(), entity_draw_submissions);
        DrawText(diagnostics.c_str(), 10, 154, 20, RAYWHITE);
        DrawText("move: WASD/arrows | quit: Q/Escape", 10, window_.height() - 30, 20, LIGHTGRAY);
    }

    void Application::try_move_player(int delta_x, int delta_y){
        Entity& player_entity = player();
        const int player_x = static_cast<int>(player_entity.world_position.x) / tile_size_;
        const int player_y = static_cast<int>(player_entity.world_position.y) / tile_size_;
        const int destination_x = player_x + delta_x;
        const int destination_y = player_y + delta_y;

        if(map_.contains(destination_x, destination_y)
            && map_.at(destination_x, destination_y).type != TileType::wall){
            player_entity.world_position = {
                static_cast<float>(destination_x * tile_size_ + tile_size_ / 2),
                static_cast<float>(destination_y * tile_size_ + tile_size_ / 2)
            };
        }
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

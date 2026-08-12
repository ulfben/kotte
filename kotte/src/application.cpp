#include "kotte/application.hpp"

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
        camera_.set_centre(player_world_centre());
    }

    void Application::render() const{
        Frame frame{background_color_};

        for(int y = 0; y < map_.height(); ++y){
            for(int x = 0; x < map_.width(); ++x){
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
            }
        }

        const Vector2 player_screen_position = camera_.world_to_screen(player_world_centre());
        DrawCircle(
            static_cast<int>(player_screen_position.x),
            static_cast<int>(player_screen_position.y),
            tile_size_ * 0.3f,
            player_color_);

        DrawFPS(GetScreenWidth() - 100, 2);
        DrawText(TextFormat("seed: %llu", static_cast<unsigned long long>(seed_)), 10, 10, 20, RAYWHITE);
        DrawText(TextFormat("tiles: %zu / %zu drawn", map_.tile_count(), map_.tile_count()), 10, 34, 20, RAYWHITE);
        DrawText("move: WASD/arrows | quit: Q/Escape", 10, window_.height() - 30, 20, LIGHTGRAY);
    }

    Vector2 Application::player_world_centre() const noexcept{
        return {
            static_cast<float>(player_x_ * tile_size_ + tile_size_ / 2),
            static_cast<float>(player_y_ * tile_size_ + tile_size_ / 2)
        };
    }

    void Application::try_move_player(int delta_x, int delta_y){
        const int destination_x = player_x_ + delta_x;
        const int destination_y = player_y_ + delta_y;

        if(map_.contains(destination_x, destination_y)
            && map_.at(destination_x, destination_y).type != TileType::wall){
            player_x_ = destination_x;
            player_y_ = destination_y;
        }
    }

    void Application::request_exit() noexcept{
        exit_requested_ = true;
    }

} //kotte

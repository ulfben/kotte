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
        , random_{seed}
        , map_{make_room(20, 12, random_)}
        , seed_{seed}{
    }

    void Application::run(){
        while(!window_.should_close() && !exit_requested_){
            update(GetFrameTime());            
            render();
        }
    }

    void Application::update(float dt){
        if(IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_Q)){
            request_exit();
        }

        if(IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)){
            try_move_player(-1, 0);
        }
        if(IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)){
            try_move_player(1, 0);
        }
        if(IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)){
            try_move_player(0, -1);
        }
        if(IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)){
            try_move_player(0, 1);
        }

        (void) dt;
    }

    void Application::render() const{
        Frame frame{background_color_};

        constexpr int tile_size = 40;
        const int map_pixel_width = map_.width() * tile_size;
        const int map_pixel_height = map_.height() * tile_size;
        const int origin_x = (window_.width() - map_pixel_width) / 2;
        const int origin_y = (window_.height() - map_pixel_height) / 2;

        constexpr Color floor_colors[]{
            Color{0x38, 0x49, 0x52, 0xff},
            Color{0x3d, 0x4f, 0x59, 0xff},
            Color{0x42, 0x55, 0x60, 0xff}
        };

        for(int y = 0; y < map_.height(); ++y){
            for(int x = 0; x < map_.width(); ++x){
                const Tile& tile = map_.at(x, y);
                const Color color = tile.type == TileType::wall
                    ? Color{0x78, 0x5f, 0x47, 0xff}
                    : floor_colors[tile.variation % 3];

                DrawRectangle(
                    origin_x + x * tile_size,
                    origin_y + y * tile_size,
                    tile_size - 1,
                    tile_size - 1,
                    color);
            }
        }

        DrawCircle(
            origin_x + player_x_ * tile_size + tile_size / 2,
            origin_y + player_y_ * tile_size + tile_size / 2,
            tile_size * 0.3f,
            Color{0xf2, 0xc1, 0x4e, 0xff});

        DrawFPS(GetScreenWidth() - 100, 2);
        DrawText(TextFormat("seed: %llu", static_cast<unsigned long long>(seed_)), 10, 10, 20, RAYWHITE);
        DrawText(TextFormat("tiles: %zu / %zu drawn", map_.tile_count(), map_.tile_count()), 10, 34, 20, RAYWHITE);
        DrawText("move: WASD/arrows | quit: Q/Escape", 10, window_.height() - 30, 20, LIGHTGRAY);
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

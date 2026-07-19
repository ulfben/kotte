#include "window.hpp"

#include <stdexcept>
#include <string>

namespace kotte
{
    DrawScopeGuard::DrawScopeGuard(Color clear) noexcept{
        BeginDrawing();
        ClearBackground(clear);
    }

    DrawScopeGuard::~DrawScopeGuard() noexcept{
        DrawFPS(GetScreenWidth() - 100, 2);
        EndDrawing();
    }

    Window::Window(int width, int height, std::string_view title, int target_fps){ 
        const std::string null_terminated_title{title};

        if(target_fps < 1){
            SetConfigFlags(FLAG_VSYNC_HINT);
        }

        InitWindow(width, height, null_terminated_title.c_str());

        if(!IsWindowReady()){
            throw std::runtime_error{"Unable to create Raylib window."};
        }

        if(target_fps > 0){
            SetTargetFPS(target_fps);           
        }     
    }

    Window::~Window() noexcept{
        CloseWindow();
    }

    bool Window::should_close() const noexcept{
        return WindowShouldClose()
            || IsKeyPressed(KEY_ESCAPE)
            || IsKeyPressed(KEY_Q);
    }
}
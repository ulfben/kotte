#include "application.hpp"

namespace kotte
{
    
    Application::Application()
        : window_{1280, 720, "Kotte"}{
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
        (void) dt;
    }

    void Application::render() const noexcept{
        Frame frame{background_color_};
        DrawFPS(GetScreenWidth() - 100, 2);
    }

    void Application::request_exit() noexcept{
        exit_requested_ = true;
    }

} //kotte
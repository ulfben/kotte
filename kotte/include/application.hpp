#pragma once
#include "window.hpp"
#include <raylib.h>

namespace kotte
{   

    class Application final{
    public:
        Application(
            int window_width,
            int window_height,
            std::string_view title,
            int target_fps = 0);

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        void run();
        void request_exit() noexcept;

    private:
        void update(float delta_time);
        void render() const noexcept;
        
        Window window_; // Constructed first, destroyed last.        
        bool exit_requested_ = false;
        Color background_color_{0x11, 0x22, 0x33, 0xff};
    };
} 
#pragma once

#include <raylib.h>

namespace kotte
{
    class Camera final{
    public:
        explicit Camera(Vector2 viewport_size) noexcept;

        void set_centre(Vector2 world_position) noexcept;
        void set_viewport_size(Vector2 viewport_size) noexcept;

        [[nodiscard]] Vector2 world_to_screen(Vector2 world_position) const noexcept;
        [[nodiscard]] Rectangle world_view() const noexcept;

    private:
        Vector2 centre_{};
        Vector2 viewport_size_{};
    };
}

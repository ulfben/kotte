#include "kotte/camera.hpp"

namespace kotte
{
    Camera::Camera(Vector2 viewport_size) noexcept
        : viewport_size_{viewport_size}{
    }

    void Camera::set_centre(Vector2 world_position) noexcept{
        centre_ = world_position;
    }

    void Camera::set_viewport_size(Vector2 viewport_size) noexcept{
        viewport_size_ = viewport_size;
    }

    Vector2 Camera::world_to_screen(Vector2 world_position) const noexcept{
        return {
            world_position.x - centre_.x + viewport_size_.x / 2.0f,
            world_position.y - centre_.y + viewport_size_.y / 2.0f
        };
    }

    Rectangle Camera::world_view() const noexcept{
        return {
            centre_.x - viewport_size_.x / 2.0f,
            centre_.y - viewport_size_.y / 2.0f,
            viewport_size_.x,
            viewport_size_.y
        };
    }
}

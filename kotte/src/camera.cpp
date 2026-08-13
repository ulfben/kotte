#include "kotte/camera.hpp"

#include <raymath.h>

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
        return world_position - centre_ + viewport_size_ / 2.0f;
    }

    Rectangle Camera::world_view() const noexcept{
        const Vector2 top_left = centre_ - viewport_size_ / 2.0f;
        return {top_left.x, top_left.y, viewport_size_.x, viewport_size_.y};
    }
}

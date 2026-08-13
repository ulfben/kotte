#pragma once

#include <cstddef>
#include <cstdint>
#include <raylib.h>
#include <vector>

namespace kotte
{
    enum class RenderLayer : std::uint8_t {
        ground,
        world
    };

    struct RenderCommand final {
        Rectangle screen_bounds{};
        Color color{};
        RenderLayer layer = RenderLayer::world;
        float depth = 0.0f;
        float roundness = 0.0f;
    };

    class Renderer final {
    public:
        void clear() noexcept;
        void submit(RenderCommand command);
        void execute() const;
        [[nodiscard]] std::size_t command_count() const noexcept;

    private:
        std::vector<RenderCommand> commands_;
    };
}

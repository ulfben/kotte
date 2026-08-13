#include "kotte/renderer.hpp"

namespace kotte
{
    void Renderer::clear() noexcept{
        commands_.clear();
    }

    void Renderer::submit(RenderCommand command){
        commands_.push_back(command);
    }

    void Renderer::execute() const{
        for(const RenderCommand& command : commands_){
            DrawRectangleRounded(command.screen_bounds, command.roundness, 6, command.color);
        }
    }

    std::size_t Renderer::command_count() const noexcept{
        return commands_.size();
    }
}

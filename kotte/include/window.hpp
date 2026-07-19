#pragma once

#include <raylib.h>
#include <string_view>

namespace kotte
{
	class DrawScopeGuard final{
	public:
		explicit DrawScopeGuard(Color clear) noexcept;
		~DrawScopeGuard() noexcept;

		DrawScopeGuard(const DrawScopeGuard&) = delete;
		DrawScopeGuard& operator=(const DrawScopeGuard&) = delete;
		DrawScopeGuard(DrawScopeGuard&&) = delete;
		DrawScopeGuard& operator=(DrawScopeGuard&&) = delete;
	};

	class Window final{
	public:
		//vsync by default, provide positive 'target_fps' to instead set an explicit software cap
		Window(int width, int height, std::string_view title, int target_fps = 0); 
		~Window() noexcept;

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;

		[[nodiscard]] bool should_close() const noexcept;
	};
}
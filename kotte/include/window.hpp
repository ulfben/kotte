#pragma once
#include <raylib.h>
#include <string_view>

namespace kotte
{
	class Frame final{
	public:
		explicit Frame(Color clear) noexcept;
		~Frame() noexcept;

		Frame(const Frame&) = delete;
		Frame& operator=(const Frame&) = delete;
		Frame(Frame&&) = delete;
		Frame& operator=(Frame&&) = delete;
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
		[[nodiscard]] int width() const noexcept;
		[[nodiscard]] int height() const noexcept;
		[[nodiscard]] Vector2 size() const noexcept;
	};
}
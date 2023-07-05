#pragma once


namespace Goril
{
	struct WindowSize
	{
		unsigned int width = 0;
		unsigned int height = 0;
	};

	class Window
	{
	public:
		virtual void Init(unsigned int width, unsigned int height) = 0;
		virtual void Shutdown() = 0;
		virtual void Present() = 0;
		virtual WindowSize GetWindowSize() const = 0;

		// Singleton code
	public:
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window& operator=(Window&&) = delete;
		Window(Window&&) = delete;

		static Window*& Get();
		
	protected:
		Window() = default;
	};
}
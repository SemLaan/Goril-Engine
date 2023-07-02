#pragma once


namespace Goril
{

	class Window
	{
	public:
		virtual void Init(unsigned int width, unsigned int height) = 0;
		virtual void Shutdown() = 0;
		virtual void Present() = 0;

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
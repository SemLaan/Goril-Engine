#include "window.h"
#include "glewwindow/glewwindow.h"

namespace Goril
{
	Window*& Window::Get() {
		static Window* instance = nullptr;
		if (!instance)
		{
			// Create a window based on the platform
			instance = new GlewWindow();
		}
		return instance;
	}
}
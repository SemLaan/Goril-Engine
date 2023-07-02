#pragma once
#include "core/window.h"

namespace Goril
{

	class GlewWindow : public Window
	{
		// Inherited via Window
		virtual void Init(unsigned int width, unsigned int height) override;
		virtual void Shutdown() override;
		virtual void Present() override;
	};
}

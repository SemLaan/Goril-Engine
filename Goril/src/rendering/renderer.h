#pragma once
#include "defines.h"


namespace GR
{

	b8 InitializeRenderer();
	void ShutdownRenderer();

	b8 BeginFrame();
	void EndFrame();

	void RecreateSwapchain();
}
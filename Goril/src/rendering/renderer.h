#pragma once
#include "defines.h"


namespace GR
{

	b8 InitializeRenderer();

	b8 UpdateRenderer();

	void ShutdownRenderer();

	void RecreateSwapchain();
}
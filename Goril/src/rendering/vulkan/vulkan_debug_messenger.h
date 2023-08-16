#pragma once
#include "defines.h"
#include "vulkan_types.h"


namespace GR
{
#ifndef GR_DIST
	VkDebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo();

	b8 CreateDebugMessenger(RendererState* state);
	void DestroyDebugMessenger(RendererState* state);
#endif // !GR_DIST
}
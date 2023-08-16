#pragma once
#include <vulkan/vulkan.h>
#include "defines.h"

namespace GR
{

	struct QueueFamilyIndices
	{
		u32 graphicsFamily;
		u32 presentFamily;
	};

	struct RendererState
	{
		VkInstance instance;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		QueueFamilyIndices queueIndices;
		VkSurfaceKHR surface;
		VkAllocationCallbacks* allocator;
#ifndef GR_DIST
		VkDebugUtilsMessengerEXT debugMessenger;
#endif // !GR_DIST
	};
}
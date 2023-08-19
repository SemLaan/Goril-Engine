#pragma once
#include "defines.h"
#include <vulkan/vulkan.h>
#include "containers/darray.h"

namespace GR
{
	void GetPlatformExtensions(Darray<const void*>* extensionNames);

	b8 PlatformCreateSurface(VkInstance instance, VkAllocationCallbacks* allocator, VkSurfaceKHR* out_surface);
}
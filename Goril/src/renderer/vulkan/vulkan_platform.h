#pragma once
#include "defines.h"
#include <vulkan/vulkan.h>
#include "containers/darray.h"


void GetPlatformExtensions(void** extensionNamesDarray);

bool PlatformCreateSurface(VkInstance instance, VkAllocationCallbacks* allocator, VkSurfaceKHR* out_surface);

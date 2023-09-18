#pragma once
#include "defines.h"
#include "vulkan_types.h"



#ifndef GR_DIST
VkDebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo();

b8 CreateDebugMessenger();
void DestroyDebugMessenger();
#endif // !GR_DIST

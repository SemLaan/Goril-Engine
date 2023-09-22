#pragma once
#include "defines.h"
#include "vulkan_types.h"



#ifndef GR_DIST
VkDebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo();

bool CreateDebugMessenger();
void DestroyDebugMessenger();
#endif // !GR_DIST

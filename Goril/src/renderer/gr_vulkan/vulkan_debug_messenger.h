#pragma once
#include "defines.h"
#include "vulkan_types.h"



#ifndef GR_DIST

VkDebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo();

bool CreateDebugMessenger();
void DestroyDebugMessenger();

#else


#endif // !GR_DIST

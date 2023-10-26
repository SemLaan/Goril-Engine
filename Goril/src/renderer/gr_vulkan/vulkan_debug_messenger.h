#pragma once
#include "defines.h"
#include "vulkan_types.h"



#ifndef GR_DIST

#define ADD_DEBUG_INSTANCE_EXTENSIONS(extensions, extensionCount) extensions[extensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME; extensionCount++
#define ADD_DEBUG_INSTANCE_LAYERS(layers, layerCount) layers[layerCount] = "VK_LAYER_KHRONOS_validation"; layerCount++

VkDebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo();

bool _CreateDebugMessenger();
void _DestroyDebugMessenger();

#define CreateDebugMessenger() _CreateDebugMessenger()
#define DestroyDebugMessenger() _DestroyDebugMessenger()

#else

#define ADD_DEBUG_INSTANCE_EXTENSIONS(extensions, extensionCount)
#define ADD_DEBUG_INSTANCE_LAYERS(layers, layerCount)
#define CreateDebugMessenger()
#define DestroyDebugMessenger()

#endif // !GR_DIST

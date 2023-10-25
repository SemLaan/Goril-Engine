#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"



SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

bool SelectPhysicalDevice(u32 requiredDeviceExtensionNameCount, const char** requiredDeviceExtensionNames);

void SelectQueueFamilies(RendererState* state);

bool CreateLogicalDevice(RendererState* state, u32 requiredDeviceExtensionNameCount, const char** requiredDeviceExtensionNames, u32 requiredLayerNameCount, const char** requiredLayerNames);

void DestroyLogicalDevice(RendererState* state);

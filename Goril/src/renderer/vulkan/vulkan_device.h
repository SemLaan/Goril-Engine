#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"



SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

bool SelectPhysicalDevice(void** requiredDeviceExtensionsDarray);

void SelectQueueFamilies(RendererState* state);

bool CreateLogicalDevice(RendererState* state, void** requiredDeviceExtensionsDarray, void** requiredDeviceLayersDarray);

void DestroyLogicalDevice(RendererState* state);

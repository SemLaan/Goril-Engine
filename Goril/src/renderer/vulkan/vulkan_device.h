#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"



SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

b8 SelectPhysicalDevice(void** requiredDeviceExtensionsDarray);

void SelectQueueFamilies(RendererState* state);

b8 CreateLogicalDevice(RendererState* state, void** requiredDeviceExtensionsDarray, void** requiredDeviceLayersDarray);

void DestroyLogicalDevice(RendererState* state);

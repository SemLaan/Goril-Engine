#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"



SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

b8 SelectPhysicalDevice(RendererState* state, const Darray<const void*>* requiredDeviceExtensions);

void SelectQueueFamilies(RendererState* state);

b8 CreateLogicalDevice(RendererState* state, const Darray<const void*>* requiredDeviceExtensions, const Darray<const void*>* requiredDeviceLayers);

void DestroyLogicalDevice(RendererState* state);

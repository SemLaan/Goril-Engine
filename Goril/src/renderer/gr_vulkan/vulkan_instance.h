#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"


bool CreateVulkanInstance(u32 requiredExtensionNameCount, const char** requiredExtensionNames, u32 requiredLayerNameCount, const char** requiredLayerNames);

void DestroyVulkanInstance();

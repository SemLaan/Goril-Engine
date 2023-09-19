#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"


bool CreateVulkanInstance(void** requiredExtensionsDarray, void** requiredLayersDarray);

void DestroyVulkanInstance();

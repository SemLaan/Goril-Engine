#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"


b8 CreateVulkanInstance(void** requiredExtensionsDarray, void** requiredLayersDarray);

void DestroyVulkanInstance();

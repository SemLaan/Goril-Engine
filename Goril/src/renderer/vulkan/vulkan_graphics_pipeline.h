#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"


b8 CreateGraphicsPipeline();
void DestroyGraphicsPipeline();

void UpdateDescriptorSets(u32 index, VulkanImage* image);

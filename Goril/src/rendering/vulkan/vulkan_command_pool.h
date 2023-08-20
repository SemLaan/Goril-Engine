#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"

namespace GR
{
	b8 CreateCommandPool();

	void DestroyCommandPool();

	b8 AllocateCommandBuffers();

	b8 RecordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex);
}
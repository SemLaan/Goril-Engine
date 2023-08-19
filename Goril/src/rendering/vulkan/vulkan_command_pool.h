#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"

namespace GR
{
	b8 CreateCommandPool(RendererState* state);

	void DestroyCommandPool(RendererState* state);

	b8 AllocateCommandBuffers(RendererState* state);

	b8 RecordCommandBuffer(RendererState* state, VkCommandBuffer commandBuffer, u32 imageIndex);
}
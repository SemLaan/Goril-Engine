#include "vulkan_command_pool.h"


namespace GR
{

	b8 CreateCommandPool(RendererState* state)
	{
		VkCommandPoolCreateInfo commandPoolCreateInfo = {};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.pNext = nullptr;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = state->queueIndices.graphicsFamily;

		if (VK_SUCCESS != vkCreateCommandPool(state->device, &commandPoolCreateInfo, state->allocator, &state->commandPool))
		{
			GRFATAL("Failed to create Vulkan command pool");
			return false;
		}

		return true;
	}

	void DestroyCommandPool(RendererState* state)
	{
		if (state->commandPool)
			vkDestroyCommandPool(state->device, state->commandPool, state->allocator);
	}
}
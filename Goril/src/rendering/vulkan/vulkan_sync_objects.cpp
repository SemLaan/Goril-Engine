#include "vulkan_sync_objects.h"


namespace GR
{
	
	b8 CreateSyncObjects(RendererState* state)
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if ((VK_SUCCESS != vkCreateSemaphore(state->device, &semaphoreCreateInfo, state->allocator, &state->imageAvailableSemaphore)) ||
			(VK_SUCCESS != vkCreateSemaphore(state->device, &semaphoreCreateInfo, state->allocator, &state->renderFinishedSemaphore)) ||
			(VK_SUCCESS != vkCreateFence(state->device, &fenceCreateInfo, state->allocator, &state->inFlightFence)))
		{
			GRFATAL("Failed to create sync objects");
			return false;
		}

		return true;
	}

	void DestroySyncObjects(RendererState* state)
	{
		if (state->imageAvailableSemaphore)
			vkDestroySemaphore(state->device, state->imageAvailableSemaphore, state->allocator);
		if (state->renderFinishedSemaphore)
			vkDestroySemaphore(state->device, state->renderFinishedSemaphore, state->allocator);
		if (state->inFlightFence)
			vkDestroyFence(state->device, state->inFlightFence, state->allocator);
	}
}
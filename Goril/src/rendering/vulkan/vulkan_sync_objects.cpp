#include "vulkan_sync_objects.h"


namespace GR
{
	
	b8 CreateSyncObjects(RendererState* state)
	{
		state->imageAvailableSemaphores = CreateDarrayWithSize<VkSemaphore>(MEM_TAG_RENDERER_SUBSYS, state->maxFramesInFlight);
		state->renderFinishedSemaphores = CreateDarrayWithSize<VkSemaphore>(MEM_TAG_RENDERER_SUBSYS, state->maxFramesInFlight);
		state->inFlightFences = CreateDarrayWithSize<VkFence>(MEM_TAG_RENDERER_SUBSYS, state->maxFramesInFlight);

		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (i32 i = 0; i < state->maxFramesInFlight; ++i)
		{
			if ((VK_SUCCESS != vkCreateSemaphore(state->device, &semaphoreCreateInfo, state->allocator, &state->imageAvailableSemaphores[i])) ||
				(VK_SUCCESS != vkCreateSemaphore(state->device, &semaphoreCreateInfo, state->allocator, &state->renderFinishedSemaphores[i])) ||
				(VK_SUCCESS != vkCreateFence(state->device, &fenceCreateInfo, state->allocator, &state->inFlightFences[i])))
			{
				GRFATAL("Failed to create sync objects");
				return false;
			}
		}

		return true;
	}

	void DestroySyncObjects(RendererState* state)
	{
		for (i32 i = 0; i < state->maxFramesInFlight; ++i)
		{
			if (state->imageAvailableSemaphores.GetRawElements())
				vkDestroySemaphore(state->device, state->imageAvailableSemaphores[i], state->allocator);
			if (state->renderFinishedSemaphores.GetRawElements())
				vkDestroySemaphore(state->device, state->renderFinishedSemaphores[i], state->allocator);
			if (state->inFlightFences.GetRawElements())
				vkDestroyFence(state->device, state->inFlightFences[i], state->allocator);
		}
		if (state->imageAvailableSemaphores.GetRawElements())
			state->imageAvailableSemaphores.Deinitialize();
		if (state->renderFinishedSemaphores.GetRawElements())
			state->renderFinishedSemaphores.Deinitialize();
		if (state->inFlightFences.GetRawElements())
			state->inFlightFences.Deinitialize();
	}
}
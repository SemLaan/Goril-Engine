#include "vulkan_sync_objects.h"


namespace GR
{
	
	b8 CreateSyncObjects()
	{
		vk_state->imageAvailableSemaphores = CreateDarrayWithSize<VkSemaphore>(MEM_TAG_RENDERER_SUBSYS, vk_state->maxFramesInFlight);
		vk_state->renderFinishedSemaphores = CreateDarrayWithSize<VkSemaphore>(MEM_TAG_RENDERER_SUBSYS, vk_state->maxFramesInFlight);
		vk_state->inFlightFences = CreateDarrayWithSize<VkFence>(MEM_TAG_RENDERER_SUBSYS, vk_state->maxFramesInFlight);

		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (i32 i = 0; i < vk_state->maxFramesInFlight; ++i)
		{
			if ((VK_SUCCESS != vkCreateSemaphore(vk_state->device, &semaphoreCreateInfo, vk_state->allocator, &vk_state->imageAvailableSemaphores[i])) ||
				(VK_SUCCESS != vkCreateSemaphore(vk_state->device, &semaphoreCreateInfo, vk_state->allocator, &vk_state->renderFinishedSemaphores[i])) ||
				(VK_SUCCESS != vkCreateFence(vk_state->device, &fenceCreateInfo, vk_state->allocator, &vk_state->inFlightFences[i])))
			{
				GRFATAL("Failed to create sync objects");
				return false;
			}
		}

		vk_state->vertexUploadSemaphore.submitValue = 0;
		vk_state->indexUploadSemaphore.submitValue = 0;
		vk_state->imageUploadSemaphore.submitValue = 0;
		vk_state->singleUseCommandBufferSemaphore.submitValue = 0;

		VkSemaphoreTypeCreateInfo semaphoreTypeInfo{};
		semaphoreTypeInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		semaphoreTypeInfo.pNext = nullptr;
		semaphoreTypeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		semaphoreTypeInfo.initialValue = 0;

		VkSemaphoreCreateInfo timelineSemaphoreCreateInfo{};
		timelineSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		timelineSemaphoreCreateInfo.pNext = &semaphoreTypeInfo;
		timelineSemaphoreCreateInfo.flags = 0;

		if (VK_SUCCESS != vkCreateSemaphore(vk_state->device, &timelineSemaphoreCreateInfo, vk_state->allocator, &vk_state->vertexUploadSemaphore.handle) ||
			VK_SUCCESS != vkCreateSemaphore(vk_state->device, &timelineSemaphoreCreateInfo, vk_state->allocator, &vk_state->indexUploadSemaphore.handle) ||
			VK_SUCCESS != vkCreateSemaphore(vk_state->device, &timelineSemaphoreCreateInfo, vk_state->allocator, &vk_state->imageUploadSemaphore.handle) ||
			VK_SUCCESS != vkCreateSemaphore(vk_state->device, &timelineSemaphoreCreateInfo, vk_state->allocator, &vk_state->singleUseCommandBufferSemaphore.handle) ||
			VK_SUCCESS != vkCreateSemaphore(vk_state->device, &timelineSemaphoreCreateInfo, vk_state->allocator, &vk_state->frameSemaphore.handle))
		{
			GRFATAL("Failed to create sync objects");
				return false;
		}

		return true;
	}

	void DestroySyncObjects()
	{
		for (i32 i = 0; i < vk_state->maxFramesInFlight; ++i)
		{
			if (vk_state->imageAvailableSemaphores.GetRawElements())
				vkDestroySemaphore(vk_state->device, vk_state->imageAvailableSemaphores[i], vk_state->allocator);
			if (vk_state->renderFinishedSemaphores.GetRawElements())
				vkDestroySemaphore(vk_state->device, vk_state->renderFinishedSemaphores[i], vk_state->allocator);
			if (vk_state->inFlightFences.GetRawElements())
				vkDestroyFence(vk_state->device, vk_state->inFlightFences[i], vk_state->allocator);
		}

		if (vk_state->vertexUploadSemaphore.handle)
			vkDestroySemaphore(vk_state->device, vk_state->vertexUploadSemaphore.handle, vk_state->allocator);
		if (vk_state->indexUploadSemaphore.handle)
			vkDestroySemaphore(vk_state->device, vk_state->indexUploadSemaphore.handle, vk_state->allocator);
		if (vk_state->imageUploadSemaphore.handle)
			vkDestroySemaphore(vk_state->device, vk_state->imageUploadSemaphore.handle, vk_state->allocator);
		if (vk_state->singleUseCommandBufferSemaphore.handle)
			vkDestroySemaphore(vk_state->device, vk_state->singleUseCommandBufferSemaphore.handle, vk_state->allocator);
		if (vk_state->frameSemaphore.handle)
			vkDestroySemaphore(vk_state->device, vk_state->frameSemaphore.handle, vk_state->allocator);

		if (vk_state->imageAvailableSemaphores.GetRawElements())
			vk_state->imageAvailableSemaphores.Deinitialize();
		if (vk_state->renderFinishedSemaphores.GetRawElements())
			vk_state->renderFinishedSemaphores.Deinitialize();
		if (vk_state->inFlightFences.GetRawElements())
			vk_state->inFlightFences.Deinitialize();
	}
}
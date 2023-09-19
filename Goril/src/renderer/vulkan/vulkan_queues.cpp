#include "vulkan_queues.h"

#include "core/logger.h"


b8 CreateQueues()
{
	// =================== Getting the device queues ======================================================
	// Present family queue
	vkGetDeviceQueue(vk_state->device, vk_state->queueIndices.presentFamily, 0, &vk_state->presentQueue);

	///TODO: get compute queue
	// Graphics, transfer and (in the future) compute queue
	vkGetDeviceQueue(vk_state->device, vk_state->queueIndices.graphicsFamily, 0, &vk_state->graphicsQueue.handle);
	vk_state->graphicsQueue.index = vk_state->queueIndices.graphicsFamily;
	vk_state->graphicsQueue.resourcesPendingDestructionDarray = (ResourceDestructionInfo*)DarrayCreate(sizeof(ResourceDestructionInfo), 20, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS); /// TODO: change allocator to renderer local allocator (when it exists)

	vkGetDeviceQueue(vk_state->device, vk_state->queueIndices.transferFamily, 0, &vk_state->transferQueue.handle);
	vk_state->transferQueue.index = vk_state->queueIndices.transferFamily;
	vk_state->transferQueue.resourcesPendingDestructionDarray = (ResourceDestructionInfo*)DarrayCreate(sizeof(ResourceDestructionInfo), 20, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS); /// TODO: change allocator to renderer local allocator (when it exists)

	// ==================== Creating command pools for each of the queue families =============================
	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = nullptr;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = vk_state->queueIndices.graphicsFamily;

	if (VK_SUCCESS != vkCreateCommandPool(vk_state->device, &commandPoolCreateInfo, vk_state->allocator, &vk_state->graphicsQueue.commandPool))
	{
		GRFATAL("Failed to create Vulkan graphics command pool");
		return false;
	}

	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	commandPoolCreateInfo.queueFamilyIndex = vk_state->queueIndices.transferFamily;

	if (VK_SUCCESS != vkCreateCommandPool(vk_state->device, &commandPoolCreateInfo, vk_state->allocator, &vk_state->transferQueue.commandPool))
	{
		GRFATAL("Failed to create Vulkan transfer command pool");
		return false;
	}

	///TODO: create compute command pool

	// Create semaphores
	vk_state->graphicsQueue.semaphore.submitValue = 0;
	vk_state->transferQueue.semaphore.submitValue = 0;

	VkSemaphoreTypeCreateInfo semaphoreTypeInfo{};
	semaphoreTypeInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	semaphoreTypeInfo.pNext = nullptr;
	semaphoreTypeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
	semaphoreTypeInfo.initialValue = 0;

	VkSemaphoreCreateInfo timelineSemaphoreCreateInfo{};
	timelineSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	timelineSemaphoreCreateInfo.pNext = &semaphoreTypeInfo;
	timelineSemaphoreCreateInfo.flags = 0;

	if (VK_SUCCESS != vkCreateSemaphore(vk_state->device, &timelineSemaphoreCreateInfo, vk_state->allocator, &vk_state->graphicsQueue.semaphore.handle) ||
		VK_SUCCESS != vkCreateSemaphore(vk_state->device, &timelineSemaphoreCreateInfo, vk_state->allocator, &vk_state->transferQueue.semaphore.handle))
	{
		GRFATAL("Failed to create sync objects");
		return false;
	}

	return true;
}

void DestroyQueues()
{
	if (vk_state->graphicsQueue.semaphore.handle)
		vkDestroySemaphore(vk_state->device, vk_state->graphicsQueue.semaphore.handle, vk_state->allocator);
	if (vk_state->transferQueue.semaphore.handle)
		vkDestroySemaphore(vk_state->device, vk_state->transferQueue.semaphore.handle, vk_state->allocator);

	if (vk_state->graphicsQueue.commandPool)
		vkDestroyCommandPool(vk_state->device, vk_state->graphicsQueue.commandPool, vk_state->allocator);

	if (vk_state->transferQueue.commandPool)
		vkDestroyCommandPool(vk_state->device, vk_state->transferQueue.commandPool, vk_state->allocator);

	if (vk_state->graphicsQueue.resourcesPendingDestructionDarray)
		DarrayDestroy(vk_state->graphicsQueue.resourcesPendingDestructionDarray);
	if (vk_state->transferQueue.resourcesPendingDestructionDarray)
		DarrayDestroy(vk_state->transferQueue.resourcesPendingDestructionDarray);
}

static void TryDestroyResourcesPendingDestructionInQueue(QueueFamily* queue)
{
	if (DarrayGetSize(queue->resourcesPendingDestructionDarray) != 0)
	{
		u64 semaphoreValue;
		vkGetSemaphoreCounterValue(vk_state->device, queue->semaphore.handle, &semaphoreValue);

		// Looping from the end of the list to the beginning so we can remove elements without ruining the loop
		for (i32 i = (i32)DarrayGetSize(queue->resourcesPendingDestructionDarray) - 1; i >= 0; --i)
		{
			if (queue->resourcesPendingDestructionDarray[i].signalValue <= semaphoreValue)
			{
				queue->resourcesPendingDestructionDarray[i].Destructor(queue->resourcesPendingDestructionDarray[i].resource);
				DarrayPopAt(queue->resourcesPendingDestructionDarray, i);
			}
		}
	}
}

void TryDestroyResourcesPendingDestruction()
{
	TryDestroyResourcesPendingDestructionInQueue(&vk_state->graphicsQueue);
	TryDestroyResourcesPendingDestructionInQueue(&vk_state->transferQueue);
}

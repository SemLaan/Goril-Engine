#include "vulkan_command_buffer.h"

#include "core/logger.h"
#include "core/asserts.h"

#define MAX_SUBMITTED_COMMAND_BUFFERS 20



b8 AllocateCommandBuffer(QueueFamily* queueFamily, CommandBuffer** out_pCommandBuffer)
{
	*out_pCommandBuffer = (CommandBuffer*)GRAlloc(sizeof(CommandBuffer), MEM_TAG_RENDERER_SUBSYS);
	CommandBuffer* commandBuffer = *out_pCommandBuffer;

	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.commandPool = queueFamily->commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;

	if (VK_SUCCESS != vkAllocateCommandBuffers(vk_state->device, &allocateInfo, &commandBuffer->handle))
	{
		GRFATAL("Failed to allocate command buffer");
		GRFree(commandBuffer);
		return false;
	}

	commandBuffer->queueFamily = queueFamily;

	return true;
}

void FreeCommandBuffer(CommandBuffer* commandBuffer)
{
	vkFreeCommandBuffers(vk_state->device, commandBuffer->queueFamily->commandPool, 1, &commandBuffer->handle);
	GRFree(commandBuffer);
}

b8 AllocateAndBeginSingleUseCommandBuffer(QueueFamily* queueFamily, CommandBuffer** out_pCommandBuffer)
{
	*out_pCommandBuffer = (CommandBuffer*)GRAlloc(sizeof(CommandBuffer), MEM_TAG_RENDERER_SUBSYS);
	CommandBuffer* commandBuffer = *out_pCommandBuffer;

	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.commandPool = queueFamily->commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;

	if (VK_SUCCESS != vkAllocateCommandBuffers(vk_state->device, &allocateInfo, &commandBuffer->handle))
	{
		GRFATAL("Failed to allocate command buffer");
		GRFree(commandBuffer);
		return false;
	}

	commandBuffer->queueFamily = queueFamily;

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	if (VK_SUCCESS != vkBeginCommandBuffer(commandBuffer->handle, &beginInfo))
	{
		GRFATAL("Failed to start recording command buffer");
		return false;
	}

	return true;
}

static void SingleUseCommandBufferDestructor(void* resource)
{
	CommandBuffer* commandBuffer = (CommandBuffer*)resource;
	vkFreeCommandBuffers(vk_state->device, commandBuffer->queueFamily->commandPool, 1, &commandBuffer->handle);
	GRFree(commandBuffer);
}

b8 EndSubmitAndFreeSingleUseCommandBuffer(CommandBuffer* commandBuffer, u32 signalSemaphoreCount /*default: 0*/, VkSemaphoreSubmitInfo* pSemaphoreSubmitInfos /*default: null*/, u64* out_signaledValue /*default: null*/)
{
	if (VK_SUCCESS != vkEndCommandBuffer(commandBuffer->handle))
	{
		GRFATAL("Failed to stop recording command buffer");
		return false;
	}

	VkCommandBufferSubmitInfo commandBufferInfo{};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	commandBufferInfo.pNext = nullptr;
	commandBufferInfo.commandBuffer = commandBuffer->handle;
	commandBufferInfo.deviceMask = 0;

	commandBuffer->queueFamily->semaphore.submitValue++;
	VkSemaphoreSubmitInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	semaphoreInfo.pNext = nullptr;
	semaphoreInfo.semaphore = commandBuffer->queueFamily->semaphore.handle;
	semaphoreInfo.value = commandBuffer->queueFamily->semaphore.submitValue;
	semaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	semaphoreInfo.deviceIndex = 0;

	VkSemaphoreSubmitInfo* semaphoreInfosDarray = (VkSemaphoreSubmitInfo*)DarrayCreate(sizeof(VkSemaphoreSubmitInfo), signalSemaphoreCount + 1, &g_Allocators->temporaryAllocator, MEM_TAG_RENDERER_SUBSYS);
	DarrayPushback(semaphoreInfosDarray, &semaphoreInfo);

	for (u32 i = 0; i < signalSemaphoreCount; ++i)
		DarrayPushback(semaphoreInfosDarray, &pSemaphoreSubmitInfos[i]);

	VkSubmitInfo2 submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submitInfo.pNext = nullptr;
	submitInfo.flags = 0;
	submitInfo.waitSemaphoreInfoCount = 0;
	submitInfo.pWaitSemaphoreInfos = nullptr;
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &commandBufferInfo;
	submitInfo.signalSemaphoreInfoCount = DarrayGetSize(semaphoreInfosDarray);
	submitInfo.pSignalSemaphoreInfos = semaphoreInfosDarray;

	VkResult result = vkQueueSubmit2(commandBuffer->queueFamily->handle, 1, &submitInfo, VK_NULL_HANDLE);

	DarrayDestroy(semaphoreInfosDarray);

	if (VK_SUCCESS != result)
	{
		GRFATAL("Failed to submit single use buffer to queue");
		return false;
	}

	ResourceDestructionInfo commandBufferDestructionInfo{};
	commandBufferDestructionInfo.resource = commandBuffer;
	commandBufferDestructionInfo.Destructor = SingleUseCommandBufferDestructor;
	commandBufferDestructionInfo.signalValue = commandBuffer->queueFamily->semaphore.submitValue;

	if (out_signaledValue)
		*out_signaledValue = commandBuffer->queueFamily->semaphore.submitValue;

	DarrayPushback(commandBuffer->queueFamily->resourcesPendingDestructionDarray, &commandBufferDestructionInfo);

	return true;
}

void ResetCommandBuffer(CommandBuffer* commandBuffer)
{
	// No reset flags, because resources attached to the command buffer don't necessarily need to be freed and this allows Vulkan to decide whats best
	vkResetCommandBuffer(commandBuffer->handle, 0);
}

b8 ResetAndBeginCommandBuffer(CommandBuffer* commandBuffer)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; /// TODO: test if the validation layer likes this or not
	beginInfo.pInheritanceInfo = nullptr;

	if (VK_SUCCESS != vkBeginCommandBuffer(commandBuffer->handle, &beginInfo))
	{
		GRFATAL("Failed to start recording command buffer");
		return false;
	}

	return true;
}

void EndCommandBuffer(CommandBuffer* commandBuffer)
{
	if (VK_SUCCESS != vkEndCommandBuffer(commandBuffer->handle))
	{
		GRFATAL("Failed to stop recording command buffer");
		return;
	}
}

b8 SubmitCommandBuffers(u32 waitSemaphoreCount, VkSemaphoreSubmitInfo* pWaitSemaphoreInfos, u32 signalSemaphoreCount, VkSemaphoreSubmitInfo* pSignalSemaphoreInfos, u32 commandBufferCount, CommandBuffer* commandBuffers, VkFence fence)
{
#ifdef GR_DEBUG
	if (commandBufferCount > MAX_SUBMITTED_COMMAND_BUFFERS)
		GRASSERT_MSG(false, "Can't submit that many command buffers at once, increase the max submitted value or reduce the amount of command buffers submitted at once");
	u32 queueFamilyIndex = commandBuffers[0].queueFamily->index;
	// Checking if all the command buffers are from the same queue family
	for (u32 i = 0; i < commandBufferCount; ++i)
	{
		if (queueFamilyIndex != commandBuffers[i].queueFamily->index)
			GRASSERT_MSG(false, "Command buffers in submit command buffers from different queue families");
	}
#endif // GR_DEBUG

	VkCommandBufferSubmitInfo* commandBufferSubmitInfosDarray = (VkCommandBufferSubmitInfo*)DarrayCreate(sizeof(VkCommandBufferSubmitInfo), commandBufferCount, &g_Allocators->temporaryAllocator, MEM_TAG_RENDERER_SUBSYS);
	for (u32 i = 0; i < commandBufferCount; ++i)
	{
		VkCommandBufferSubmitInfo commandBufferInfo{};
		commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		commandBufferInfo.pNext = nullptr;
		commandBufferInfo.commandBuffer = commandBuffers[i].handle;
		commandBufferInfo.deviceMask = 0;
		DarrayPushback(commandBufferSubmitInfosDarray, &commandBufferInfo);
	}

	commandBuffers[0].queueFamily->semaphore.submitValue++;
	VkSemaphoreSubmitInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	semaphoreInfo.pNext = nullptr;
	semaphoreInfo.semaphore = commandBuffers[0].queueFamily->semaphore.handle;
	semaphoreInfo.value = commandBuffers[0].queueFamily->semaphore.submitValue;
	semaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	semaphoreInfo.deviceIndex = 0;

	VkSemaphoreSubmitInfo* semaphoreInfosDarray = (VkSemaphoreSubmitInfo*)DarrayCreate(sizeof(VkSemaphoreSubmitInfo), signalSemaphoreCount + 1, &g_Allocators->temporaryAllocator, MEM_TAG_RENDERER_SUBSYS);
	DarrayPushback(semaphoreInfosDarray, &semaphoreInfo);

	for (u32 i = 0; i < signalSemaphoreCount; ++i)
		DarrayPushback(semaphoreInfosDarray, &pSignalSemaphoreInfos[i]);

	VkSubmitInfo2 submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submitInfo.pNext = nullptr;
	submitInfo.flags = 0;
	submitInfo.waitSemaphoreInfoCount = waitSemaphoreCount;
	submitInfo.pWaitSemaphoreInfos = pWaitSemaphoreInfos;
	submitInfo.commandBufferInfoCount = commandBufferCount;
	submitInfo.pCommandBufferInfos = commandBufferSubmitInfosDarray;
	submitInfo.signalSemaphoreInfoCount = DarrayGetSize(semaphoreInfosDarray);
	submitInfo.pSignalSemaphoreInfos = semaphoreInfosDarray;

	if (VK_SUCCESS != vkQueueSubmit2(commandBuffers[0].queueFamily->handle, 1, &submitInfo, fence))
	{
		DarrayDestroy(semaphoreInfosDarray);
		DarrayDestroy(commandBufferSubmitInfosDarray);
		GRERROR("Failed to submit queue");
		return false;
	}

	DarrayDestroy(semaphoreInfosDarray);
	DarrayDestroy(commandBufferSubmitInfosDarray);

	return true;
}

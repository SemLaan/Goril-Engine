#include "vulkan_command_buffer.h"

#define MAX_SUBMITTED_COMMAND_BUFFERS 20

namespace GR
{

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

	b8 SubmitCommandBuffers(u32 waitSemaphoreCount, VkSemaphore* pWaitSemaphores, VkPipelineStageFlags* pWaitDstStageMask, u32 signalSemaphoreCount, VkSemaphore* pSignalSemaphores, u32 commandBufferCount, CommandBuffer* commandBuffers, VkFence fence)
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

		VkCommandBuffer commandBufferHandles[MAX_SUBMITTED_COMMAND_BUFFERS];
		for (u32 i = 0; i < commandBufferCount; ++i)
		{
			commandBufferHandles[i] = commandBuffers[i].handle;
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = waitSemaphoreCount;
		submitInfo.pWaitSemaphores = pWaitSemaphores;
		submitInfo.pWaitDstStageMask = pWaitDstStageMask;
		submitInfo.commandBufferCount = commandBufferCount;
		submitInfo.pCommandBuffers = commandBufferHandles;
		submitInfo.signalSemaphoreCount = signalSemaphoreCount;
		submitInfo.pSignalSemaphores = pSignalSemaphores;

		if (VK_SUCCESS != vkQueueSubmit(commandBuffers[0].queueFamily->handle, 1, &submitInfo, fence))
		{
			GRERROR("Failed to submit queue");
			return false;
		}

		return true;
	}
}
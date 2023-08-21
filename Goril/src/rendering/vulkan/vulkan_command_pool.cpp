#include "vulkan_command_pool.h"


namespace GR
{

	b8 CreateCommandPool()
	{
		VkCommandPoolCreateInfo commandPoolCreateInfo = {};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.pNext = nullptr;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = vk_state->queueIndices.graphicsFamily;

		if (VK_SUCCESS != vkCreateCommandPool(vk_state->device, &commandPoolCreateInfo, vk_state->allocator, &vk_state->graphicsCommandPool))
		{
			GRFATAL("Failed to create Vulkan graphics command pool");
			return false;
		}

		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		commandPoolCreateInfo.queueFamilyIndex = vk_state->queueIndices.transferFamily;

		if (VK_SUCCESS != vkCreateCommandPool(vk_state->device, &commandPoolCreateInfo, vk_state->allocator, &vk_state->transferCommandPool))
		{
			GRFATAL("Failed to create Vulkan transfer command pool");
			return false;
		}

		return true;
	}

	void DestroyCommandPool()
	{
		if (vk_state->graphicsCommandPool)
			vkDestroyCommandPool(vk_state->device, vk_state->graphicsCommandPool, vk_state->allocator);

		if (vk_state->transferCommandPool)
			vkDestroyCommandPool(vk_state->device, vk_state->transferCommandPool, vk_state->allocator);

		if (vk_state->commandBuffers.GetRawElements())
			vk_state->commandBuffers.Deinitialize();
	}

	b8 AllocateCommandBuffers()
	{
		vk_state->commandBuffers = CreateDarrayWithSize<VkCommandBuffer>(MEM_TAG_RENDERER_SUBSYS, vk_state->maxFramesInFlight);

		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.commandPool = vk_state->graphicsCommandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = vk_state->maxFramesInFlight;

		if (VK_SUCCESS != vkAllocateCommandBuffers(vk_state->device, &allocateInfo, vk_state->commandBuffers.GetRawElements()))
		{
			GRFATAL("Failed to allocate command buffer(s)");
			return false;
		}

		return true;
	}

	b8 RecordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (VK_SUCCESS != vkBeginCommandBuffer(commandBuffer, &beginInfo))
		{
			GRFATAL("Beginning command buffer failed");
			return false;
		}

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

		VkRenderPassBeginInfo renderpassBeginInfo = {};
		renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpassBeginInfo.pNext = nullptr;
		renderpassBeginInfo.renderPass = vk_state->renderpass;
		renderpassBeginInfo.framebuffer = vk_state->swapchainFramebuffers[imageIndex];
		renderpassBeginInfo.renderArea.offset = { 0, 0 };
		renderpassBeginInfo.renderArea.extent = vk_state->swapchainExtent;
		renderpassBeginInfo.clearValueCount = 1;
		renderpassBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_state->graphicsPipeline);

		// Viewport and scissor
		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = (f32)vk_state->swapchainExtent.width;
		viewport.height = (f32)vk_state->swapchainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = vk_state->swapchainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		VulkanVertexBuffer* vertexBuffer = (VulkanVertexBuffer*)vk_state->vertexBuffer->internalState;
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer->handle, offsets);

		VulkanIndexBuffer* indexBuffer = (VulkanIndexBuffer*)vk_state->indexBuffer->internalState;
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer->handle, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffer, (u32)indexBuffer->indexCount, 1, 0, 0, 0);
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		if (VK_SUCCESS != vkEndCommandBuffer(commandBuffer))
		{
			GRFATAL("Failed to record command buffer");
			return false;
		}

		return true;
	}
}
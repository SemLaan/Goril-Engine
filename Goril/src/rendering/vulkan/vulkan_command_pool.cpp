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

		if (state->commandBuffers.GetRawElements())
			state->commandBuffers.Deinitialize();
	}

	b8 AllocateCommandBuffers(RendererState* state)
	{
		state->commandBuffers = CreateDarrayWithSize<VkCommandBuffer>(MEM_TAG_RENDERER_SUBSYS, state->maxFramesInFlight);

		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.commandPool = state->commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = state->maxFramesInFlight;

		if (VK_SUCCESS != vkAllocateCommandBuffers(state->device, &allocateInfo, state->commandBuffers.GetRawElements()))
		{
			GRFATAL("Failed to allocate command buffer(s)");
			return false;
		}

		return true;
	}

	b8 RecordCommandBuffer(RendererState* state, VkCommandBuffer commandBuffer, u32 imageIndex)
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
		renderpassBeginInfo.renderPass = state->renderpass;
		renderpassBeginInfo.framebuffer = state->swapchainFramebuffers[imageIndex];
		renderpassBeginInfo.renderArea.offset = { 0, 0 };
		renderpassBeginInfo.renderArea.extent = state->swapchainExtent;
		renderpassBeginInfo.clearValueCount = 1;
		renderpassBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state->graphicsPipeline);

		// Viewport and scissor
		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = (f32)state->swapchainExtent.width;
		viewport.height = (f32)state->swapchainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = state->swapchainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

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
#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"

namespace GR
{

	///TODO: replace
	b8 RecordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex);


	b8 AllocateCommandBuffer(QueueFamily* queueFamily, CommandBuffer* out_commandBuffer);
	void FreeCommandBuffer(CommandBuffer* commandBuffer);

	/// <summary>
	/// Resets the command buffer, only supported if the command buffer was allocated from a command pool with the VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag.
	/// </summary>
	/// <param name="commandBuffer"></param>
	void ResetCommandBuffer(CommandBuffer* commandBuffer);
	b8 ResetAndBeginCommandBuffer(CommandBuffer* commandBuffer);
	void EndCommandBuffer(CommandBuffer* commandBuffer);
	b8 SubmitCommandBuffers(u32 count, CommandBuffer* commandBuffers);
}
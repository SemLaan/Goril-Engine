#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"




b8 AllocateCommandBuffer(QueueFamily* queueFamily, CommandBuffer** out_pCommandBuffer);
void FreeCommandBuffer(CommandBuffer* commandBuffer);

b8 AllocateAndBeginSingleUseCommandBuffer(QueueFamily* queueFamily, CommandBuffer** out_pCommandBuffer);
b8 EndSubmitAndFreeSingleUseCommandBuffer(CommandBuffer* commandBuffer, u32 signalSemaphoreCount = 0, VkSemaphoreSubmitInfo* pSemaphoreSubmitInfos = nullptr, u64* out_signaledValue = nullptr);

/// <summary>
/// Resets the command buffer, only supported if the command buffer was allocated from a command pool with the VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag.
/// </summary>
/// <param name="commandBuffer"></param>
void ResetCommandBuffer(CommandBuffer* commandBuffer);
b8 ResetAndBeginCommandBuffer(CommandBuffer* commandBuffer);
void EndCommandBuffer(CommandBuffer* commandBuffer);
b8 SubmitCommandBuffers(u32 waitSemaphoreCount, VkSemaphoreSubmitInfo* pWaitSemaphoreInfos, u32 signalSemaphoreCount, VkSemaphoreSubmitInfo* pSignalSemaphoreInfos, u32 commandBufferCount, CommandBuffer* commandBuffers, VkFence fence);

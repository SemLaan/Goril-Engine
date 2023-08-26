// This file implements both of these headers!
#include "../buffer.h"
#include "vulkan_buffer.h"

#include "vulkan_types.h"
#include "vulkan_command_buffer.h"

namespace GR
{

	u32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags requiredFlags)
	{
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
		vkGetPhysicalDeviceMemoryProperties(vk_state->physicalDevice, &deviceMemoryProperties);

		for (u32 i = 0; i < deviceMemoryProperties.memoryTypeCount; ++i)
		{
			if (typeFilter & (1 << i) && (deviceMemoryProperties.memoryTypes[i].propertyFlags & requiredFlags) == requiredFlags)
			{
				return i;
			}
		}

		GRASSERT(false);
		return 0;
	}

	static void CopyBufferAndTransitionQueue(VkBuffer dstBuffer, VkBuffer srcBuffer, u32 signalSemaphoreCount, VkSemaphoreSubmitInfo* pSemaphoreInfos, VkDependencyInfo* pDependencyInfo, VkDeviceSize size, u64* out_signaledValue)
	{
		CommandBuffer* transferCommandBuffer;
		AllocateAndBeginSingleUseCommandBuffer(&vk_state->transferQueue, &transferCommandBuffer);

		VkBufferCopy copyRegion{};
		copyRegion.dstOffset = 0;
		copyRegion.srcOffset = 0;
		copyRegion.size = size;

		vkCmdCopyBuffer(transferCommandBuffer->handle, srcBuffer, dstBuffer, 1, &copyRegion);

		if (pDependencyInfo)
			vkCmdPipelineBarrier2(transferCommandBuffer->handle, pDependencyInfo);

		EndSubmitAndFreeSingleUseCommandBuffer(transferCommandBuffer, signalSemaphoreCount, pSemaphoreInfos, out_signaledValue);
	}

	b8 CreateBuffer(VkDeviceSize size, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkBuffer* out_buffer, VkDeviceMemory* out_memory)
	{
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = nullptr;
		bufferCreateInfo.flags = 0;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = bufferUsageFlags;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferCreateInfo.queueFamilyIndexCount = 0;
		bufferCreateInfo.pQueueFamilyIndices = nullptr;

		if (VK_SUCCESS != vkCreateBuffer(vk_state->device, &bufferCreateInfo, vk_state->allocator, out_buffer))
		{
			GRFATAL("Buffer creation failed");
			return false;
		}

		VkMemoryRequirements stagingMemoryRequirements;
		vkGetBufferMemoryRequirements(vk_state->device, *out_buffer, &stagingMemoryRequirements);

		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.allocationSize = stagingMemoryRequirements.size;
		allocateInfo.memoryTypeIndex = FindMemoryType(stagingMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (VK_SUCCESS != vkAllocateMemory(vk_state->device, &allocateInfo, vk_state->allocator, out_memory))
		{
			GRFATAL("Vulkan device memory allocation failed");
			return false;
		}

		vkBindBufferMemory(vk_state->device, *out_buffer, *out_memory, 0);

		return true;
	}

	static void OneTimeBufferDestructor(void* resource)
	{
		vkDestroyBuffer(vk_state->device, (VkBuffer)resource, vk_state->allocator);
	}

	static void OneTimeMemoryDestructor(void* resource)
	{
		vkFreeMemory(vk_state->device, (VkDeviceMemory)resource, vk_state->allocator);
	}

	VertexBuffer CreateVertexBuffer(void* vertices, size_t size)
	{
		VertexBuffer clientBuffer;
		clientBuffer.internalState = GRAlloc(sizeof(VulkanVertexBuffer), MEM_TAG_VERTEX_BUFFER);
		VulkanVertexBuffer* buffer = (VulkanVertexBuffer*)clientBuffer.internalState;
		buffer->size = size;

		// ================ Staging buffer =========================
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo stagingBufferCreateInfo{};
		stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferCreateInfo.pNext = nullptr;
		stagingBufferCreateInfo.flags = 0;
		stagingBufferCreateInfo.size = buffer->size;
		stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		stagingBufferCreateInfo.queueFamilyIndexCount = 0;
		stagingBufferCreateInfo.pQueueFamilyIndices = nullptr;

		if (VK_SUCCESS != vkCreateBuffer(vk_state->device, &stagingBufferCreateInfo, vk_state->allocator, &stagingBuffer))
			GRASSERT(false);

		VkMemoryRequirements stagingMemoryRequirements;
		vkGetBufferMemoryRequirements(vk_state->device, stagingBuffer, &stagingMemoryRequirements);
		
		VkMemoryAllocateInfo stagingAllocateInfo{};
		stagingAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		stagingAllocateInfo.pNext = nullptr;
		stagingAllocateInfo.allocationSize = stagingMemoryRequirements.size;
		stagingAllocateInfo.memoryTypeIndex = FindMemoryType(stagingMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (VK_SUCCESS != vkAllocateMemory(vk_state->device, &stagingAllocateInfo, vk_state->allocator, &stagingMemory))
			GRASSERT(false);

		vkBindBufferMemory(vk_state->device, stagingBuffer, stagingMemory, 0);

		// ================= copying data into staging buffer ===============================
		void* data;
		vkMapMemory(vk_state->device, stagingMemory, 0, stagingBufferCreateInfo.size, 0, &data);
		MemCopy(data, vertices, (size_t)stagingBufferCreateInfo.size);
		vkUnmapMemory(vk_state->device, stagingMemory);

		// ================= creating the actual buffer =========================
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = nullptr;
		bufferCreateInfo.flags = 0;
		bufferCreateInfo.size = buffer->size;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferCreateInfo.queueFamilyIndexCount = 2;
		u32 queueFamilyIndices[2] = { vk_state->queueIndices.graphicsFamily, vk_state->queueIndices.transferFamily };
		bufferCreateInfo.pQueueFamilyIndices = queueFamilyIndices;

		if (VK_SUCCESS != vkCreateBuffer(vk_state->device, &bufferCreateInfo, vk_state->allocator, &buffer->handle))
			GRASSERT(false);

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(vk_state->device, buffer->handle, &memoryRequirements);

		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (VK_SUCCESS != vkAllocateMemory(vk_state->device, &allocateInfo, vk_state->allocator, &buffer->memory))
			GRASSERT(false);

		vkBindBufferMemory(vk_state->device, buffer->handle, buffer->memory, 0);

		VkBufferMemoryBarrier2 releaseBufferInfo{};
		releaseBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
		releaseBufferInfo.pNext = nullptr;
		releaseBufferInfo.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		releaseBufferInfo.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		releaseBufferInfo.dstStageMask = 0;  // IGNORED because it is a queue family release operation
		releaseBufferInfo.dstAccessMask = 0; // IGNORED because it is a queue family release operation
		releaseBufferInfo.srcQueueFamilyIndex = vk_state->transferQueue.index;
		releaseBufferInfo.dstQueueFamilyIndex = vk_state->graphicsQueue.index;
		releaseBufferInfo.buffer = buffer->handle;
		releaseBufferInfo.offset = 0;
		releaseBufferInfo.size = buffer->size;

		VkDependencyInfo releaseDependencyInfo{};
		releaseDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		releaseDependencyInfo.pNext = nullptr;
		releaseDependencyInfo.dependencyFlags = 0;
		releaseDependencyInfo.memoryBarrierCount = 0;
		releaseDependencyInfo.pMemoryBarriers = nullptr;
		releaseDependencyInfo.bufferMemoryBarrierCount = 1;
		releaseDependencyInfo.pBufferMemoryBarriers = &releaseBufferInfo;
		releaseDependencyInfo.imageMemoryBarrierCount = 0;
		releaseDependencyInfo.pImageMemoryBarriers = nullptr;

		vk_state->vertexUploadSemaphore.submitValue++;
		VkSemaphoreSubmitInfo semaphoreSubmitInfo{};
		semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		semaphoreSubmitInfo.pNext = nullptr;
		semaphoreSubmitInfo.semaphore = vk_state->vertexUploadSemaphore.handle;
		semaphoreSubmitInfo.value = vk_state->vertexUploadSemaphore.submitValue;
		semaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
		semaphoreSubmitInfo.deviceIndex = 0;

		u64 signaledValue;
		CopyBufferAndTransitionQueue(buffer->handle, stagingBuffer, 1, &semaphoreSubmitInfo, &releaseDependencyInfo, bufferCreateInfo.size, &signaledValue);

		VkDependencyInfo* acquireDependencyInfo = (VkDependencyInfo*)GRAlloc(sizeof(VkDependencyInfo) + sizeof(VkBufferMemoryBarrier2), MEM_TAG_RENDERER_SUBSYS);
		VkBufferMemoryBarrier2* acquireBufferInfo = (VkBufferMemoryBarrier2*)(acquireDependencyInfo + 1);
		
		acquireBufferInfo->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
		acquireBufferInfo->pNext = nullptr;
		acquireBufferInfo->srcStageMask = 0;  // IGNORED because it is a queue family release operation
		acquireBufferInfo->srcAccessMask = 0; // IGNORED because it is a queue family release operation
		acquireBufferInfo->dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
		acquireBufferInfo->dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
		acquireBufferInfo->srcQueueFamilyIndex = vk_state->transferQueue.index;
		acquireBufferInfo->dstQueueFamilyIndex = vk_state->graphicsQueue.index;
		acquireBufferInfo->buffer = buffer->handle;
		acquireBufferInfo->offset = 0;
		acquireBufferInfo->size = buffer->size;

		acquireDependencyInfo->sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		acquireDependencyInfo->pNext = nullptr;
		acquireDependencyInfo->dependencyFlags = 0;
		acquireDependencyInfo->memoryBarrierCount = 0;
		acquireDependencyInfo->pMemoryBarriers = nullptr;
		acquireDependencyInfo->bufferMemoryBarrierCount = 1;
		acquireDependencyInfo->pBufferMemoryBarriers = acquireBufferInfo;
		acquireDependencyInfo->imageMemoryBarrierCount = 0;
		acquireDependencyInfo->pImageMemoryBarriers = nullptr;

		vk_state->requestedQueueAcquisitionOperations.Pushback(acquireDependencyInfo);

		InFlightTemporaryResource inFlightBuffer{};
		inFlightBuffer.resource = stagingBuffer;
		inFlightBuffer.Destructor = OneTimeBufferDestructor;
		inFlightBuffer.signalValue = signaledValue;

		InFlightTemporaryResource inFlightMemory{};
		inFlightMemory.resource = stagingMemory;
		inFlightMemory.Destructor = OneTimeMemoryDestructor;
		inFlightMemory.signalValue = signaledValue;

		vk_state->singleUseCommandBufferResourcesInFlight.Pushback(inFlightBuffer);
		vk_state->singleUseCommandBufferResourcesInFlight.Pushback(inFlightMemory);

		return clientBuffer;
	}

	void DestroyVertexBuffer(VertexBuffer clientBuffer)
	{
		VulkanVertexBuffer* buffer = (VulkanVertexBuffer*)clientBuffer.internalState;

		vkDestroyBuffer(vk_state->device, buffer->handle, vk_state->allocator);
		vkFreeMemory(vk_state->device, buffer->memory, vk_state->allocator);

		GRFree(buffer);
	}

	IndexBuffer CreateIndexBuffer(u32* indices, size_t indexCount)
	{
		IndexBuffer clientBuffer;
		clientBuffer.internalState = GRAlloc(sizeof(VulkanIndexBuffer), MEM_TAG_INDEX_BUFFER);
		VulkanIndexBuffer* buffer = (VulkanIndexBuffer*)clientBuffer.internalState;
		buffer->size = indexCount * sizeof(u32);
		buffer->indexCount = indexCount;

		// ================ Staging buffer =========================
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo stagingBufferCreateInfo{};
		stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferCreateInfo.pNext = nullptr;
		stagingBufferCreateInfo.flags = 0;
		stagingBufferCreateInfo.size = buffer->size;
		stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		stagingBufferCreateInfo.queueFamilyIndexCount = 0;
		stagingBufferCreateInfo.pQueueFamilyIndices = nullptr;

		if (VK_SUCCESS != vkCreateBuffer(vk_state->device, &stagingBufferCreateInfo, vk_state->allocator, &stagingBuffer))
			GRASSERT(false);

		VkMemoryRequirements stagingMemoryRequirements;
		vkGetBufferMemoryRequirements(vk_state->device, stagingBuffer, &stagingMemoryRequirements);

		VkMemoryAllocateInfo stagingAllocateInfo{};
		stagingAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		stagingAllocateInfo.pNext = nullptr;
		stagingAllocateInfo.allocationSize = stagingMemoryRequirements.size;
		stagingAllocateInfo.memoryTypeIndex = FindMemoryType(stagingMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (VK_SUCCESS != vkAllocateMemory(vk_state->device, &stagingAllocateInfo, vk_state->allocator, &stagingMemory))
			GRASSERT(false);

		vkBindBufferMemory(vk_state->device, stagingBuffer, stagingMemory, 0);

		// ================= copying data into staging buffer ===============================
		void* data;
		vkMapMemory(vk_state->device, stagingMemory, 0, stagingBufferCreateInfo.size, 0, &data);
		MemCopy(data, indices, (size_t)buffer->size);
		vkUnmapMemory(vk_state->device, stagingMemory);

		// ================= creating the actual buffer =========================
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = nullptr;
		bufferCreateInfo.flags = 0;
		bufferCreateInfo.size = buffer->size;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferCreateInfo.queueFamilyIndexCount = 2;
		u32 queueFamilyIndices[2] = { vk_state->queueIndices.graphicsFamily, vk_state->queueIndices.transferFamily };
		bufferCreateInfo.pQueueFamilyIndices = queueFamilyIndices;

		if (VK_SUCCESS != vkCreateBuffer(vk_state->device, &bufferCreateInfo, vk_state->allocator, &buffer->handle))
			GRASSERT(false);

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(vk_state->device, buffer->handle, &memoryRequirements);

		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (VK_SUCCESS != vkAllocateMemory(vk_state->device, &allocateInfo, vk_state->allocator, &buffer->memory))
			GRASSERT(false);

		vkBindBufferMemory(vk_state->device, buffer->handle, buffer->memory, 0);

		VkBufferMemoryBarrier2 releaseBufferInfo{};
		releaseBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
		releaseBufferInfo.pNext = nullptr;
		releaseBufferInfo.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		releaseBufferInfo.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		releaseBufferInfo.dstStageMask = 0;  // IGNORED because it is a queue family release operation
		releaseBufferInfo.dstAccessMask = 0; // IGNORED because it is a queue family release operation
		releaseBufferInfo.srcQueueFamilyIndex = vk_state->transferQueue.index;
		releaseBufferInfo.dstQueueFamilyIndex = vk_state->graphicsQueue.index;
		releaseBufferInfo.buffer = buffer->handle;
		releaseBufferInfo.offset = 0;
		releaseBufferInfo.size = buffer->size;

		VkDependencyInfo releaseDependencyInfo{};
		releaseDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		releaseDependencyInfo.pNext = nullptr;
		releaseDependencyInfo.dependencyFlags = 0;
		releaseDependencyInfo.memoryBarrierCount = 0;
		releaseDependencyInfo.pMemoryBarriers = nullptr;
		releaseDependencyInfo.bufferMemoryBarrierCount = 1;
		releaseDependencyInfo.pBufferMemoryBarriers = &releaseBufferInfo;
		releaseDependencyInfo.imageMemoryBarrierCount = 0;
		releaseDependencyInfo.pImageMemoryBarriers = nullptr;

		vk_state->indexUploadSemaphore.submitValue++;
		VkSemaphoreSubmitInfo semaphoreSubmitInfo{};
		semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		semaphoreSubmitInfo.pNext = nullptr;
		semaphoreSubmitInfo.semaphore = vk_state->indexUploadSemaphore.handle;
		semaphoreSubmitInfo.value = vk_state->indexUploadSemaphore.submitValue;
		semaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
		semaphoreSubmitInfo.deviceIndex = 0;

		u64 signaledValue;
		CopyBufferAndTransitionQueue(buffer->handle, stagingBuffer, 1, &semaphoreSubmitInfo, &releaseDependencyInfo, buffer->size, &signaledValue);

		VkDependencyInfo* acquireDependencyInfo = (VkDependencyInfo*)GRAlloc(sizeof(VkDependencyInfo) + sizeof(VkBufferMemoryBarrier2), MEM_TAG_RENDERER_SUBSYS);
		VkBufferMemoryBarrier2* acquireBufferInfo = (VkBufferMemoryBarrier2*)(acquireDependencyInfo + 1);

		acquireBufferInfo->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
		acquireBufferInfo->pNext = nullptr;
		acquireBufferInfo->srcStageMask = 0;  // IGNORED because it is a queue family release operation
		acquireBufferInfo->srcAccessMask = 0; // IGNORED because it is a queue family release operation
		acquireBufferInfo->dstStageMask = VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
		acquireBufferInfo->dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
		acquireBufferInfo->srcQueueFamilyIndex = vk_state->transferQueue.index;
		acquireBufferInfo->dstQueueFamilyIndex = vk_state->graphicsQueue.index;
		acquireBufferInfo->buffer = buffer->handle;
		acquireBufferInfo->offset = 0;
		acquireBufferInfo->size = buffer->size;

		acquireDependencyInfo->sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		acquireDependencyInfo->pNext = nullptr;
		acquireDependencyInfo->dependencyFlags = 0;
		acquireDependencyInfo->memoryBarrierCount = 0;
		acquireDependencyInfo->pMemoryBarriers = nullptr;
		acquireDependencyInfo->bufferMemoryBarrierCount = 1;
		acquireDependencyInfo->pBufferMemoryBarriers = acquireBufferInfo;
		acquireDependencyInfo->imageMemoryBarrierCount = 0;
		acquireDependencyInfo->pImageMemoryBarriers = nullptr;

		vk_state->requestedQueueAcquisitionOperations.Pushback(acquireDependencyInfo);

		InFlightTemporaryResource inFlightBuffer{};
		inFlightBuffer.resource = stagingBuffer;
		inFlightBuffer.Destructor = OneTimeBufferDestructor;
		inFlightBuffer.signalValue = signaledValue;

		InFlightTemporaryResource inFlightMemory{};
		inFlightMemory.resource = stagingMemory;
		inFlightMemory.Destructor = OneTimeMemoryDestructor;
		inFlightMemory.signalValue = signaledValue;

		vk_state->singleUseCommandBufferResourcesInFlight.Pushback(inFlightBuffer);
		vk_state->singleUseCommandBufferResourcesInFlight.Pushback(inFlightMemory);

		return clientBuffer;
	}

	void DestroyIndexBuffer(IndexBuffer clientBuffer)
	{
		VulkanIndexBuffer* buffer = (VulkanIndexBuffer*)clientBuffer.internalState;

		vkDestroyBuffer(vk_state->device, buffer->handle, vk_state->allocator);
		vkFreeMemory(vk_state->device, buffer->memory, vk_state->allocator);

		GRFree(buffer);
	}
}
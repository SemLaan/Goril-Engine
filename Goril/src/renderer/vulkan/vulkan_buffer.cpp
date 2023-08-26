// This file implements both of these headers!
#include "../buffer.h"
#include "vulkan_buffer.h"

#include "vulkan_types.h"
#include "vulkan_command_buffer.h"

namespace GR
{

	static void CopyBufferAndTransitionQueue(VkBuffer dstBuffer, VkBuffer srcBuffer, VkDependencyInfo* pDependencyInfo, VkDeviceSize size, u64* out_signaledValue);


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

	static void CopyBufferAndTransitionQueue(VkBuffer dstBuffer, VkBuffer srcBuffer, VkDependencyInfo* pDependencyInfo, VkDeviceSize size, u64* out_signaledValue)
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

		EndSubmitAndFreeSingleUseCommandBuffer(transferCommandBuffer, out_signaledValue);
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
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT; /// TODO: use memory barriers instead of concurrent
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

		VkDependencyInfo dependencyInfo{};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pNext = nullptr;
		dependencyInfo.dependencyFlags;
		dependencyInfo.memoryBarrierCount;
		dependencyInfo.pMemoryBarriers;
		dependencyInfo.bufferMemoryBarrierCount;
		dependencyInfo.pBufferMemoryBarriers;
		dependencyInfo.imageMemoryBarrierCount;
		dependencyInfo.pImageMemoryBarriers;
		/// TODO: make this use dependency info

		u64 signaledValue;
		CopyBufferAndTransitionQueue(buffer->handle, stagingBuffer, nullptr, bufferCreateInfo.size, &signaledValue);

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
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT; /// TODO: use memory barriers instead of concurrent
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

		VkDependencyInfo dependencyInfo{};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pNext = nullptr;
		dependencyInfo.dependencyFlags;
		dependencyInfo.memoryBarrierCount;
		dependencyInfo.pMemoryBarriers;
		dependencyInfo.bufferMemoryBarrierCount;
		dependencyInfo.pBufferMemoryBarriers;
		dependencyInfo.imageMemoryBarrierCount;
		dependencyInfo.pImageMemoryBarriers;
		/// TODO: make this use dependency info

		u64 signaledValue;
		CopyBufferAndTransitionQueue(buffer->handle, stagingBuffer, nullptr, buffer->size, &signaledValue);

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
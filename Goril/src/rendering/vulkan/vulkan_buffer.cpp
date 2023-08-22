#include "../buffer.h"
#include "vulkan_types.h"

namespace GR
{

	static u32 FindMemoryType(VkPhysicalDevice physicalDevice, u32 typeFilter, VkMemoryPropertyFlags requiredFlags)
	{
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

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

	static void CopyBuffer(VkBuffer dstBuffer, VkBuffer srcBuffer, VkDeviceSize size)
	{
		VkCommandBufferAllocateInfo commandBufferAllocInfo{};
		commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocInfo.pNext = nullptr;
		commandBufferAllocInfo.commandPool = vk_state->transferQueue.commandPool;
		commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocInfo.commandBufferCount = 1;

		VkCommandBuffer transferCommandBuffer;

		if (VK_SUCCESS != vkAllocateCommandBuffers(vk_state->device, &commandBufferAllocInfo, &transferCommandBuffer))
		{
			GRFATAL("Failed to allocate command buffer(s)");
			GRASSERT(false);
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		vkBeginCommandBuffer(transferCommandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.dstOffset = 0;
		copyRegion.srcOffset = 0;
		copyRegion.size = size;

		vkCmdCopyBuffer(transferCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		vkEndCommandBuffer(transferCommandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &transferCommandBuffer;

		vkQueueSubmit(vk_state->transferQueue.handle, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(vk_state->transferQueue.handle);

		vkFreeCommandBuffers(vk_state->device, vk_state->transferQueue.commandPool, 1, &transferCommandBuffer);
	}

	VertexBuffer* CreateVertexBuffer(void* vertices, size_t size)
	{
		VertexBuffer* clientBuffer = (VertexBuffer*)GRAlloc(sizeof(VertexBuffer) + sizeof(VulkanVertexBuffer), MEM_TAG_RENDERER_SUBSYS);
		clientBuffer->internalState = clientBuffer + 1;
		VulkanVertexBuffer* buffer = (VulkanVertexBuffer*)clientBuffer->internalState;
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
		stagingAllocateInfo.memoryTypeIndex = FindMemoryType(vk_state->physicalDevice, stagingMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

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
		allocateInfo.memoryTypeIndex = FindMemoryType(vk_state->physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (VK_SUCCESS != vkAllocateMemory(vk_state->device, &allocateInfo, vk_state->allocator, &buffer->memory))
			GRASSERT(false);

		vkBindBufferMemory(vk_state->device, buffer->handle, buffer->memory, 0);

		CopyBuffer(buffer->handle, stagingBuffer, bufferCreateInfo.size);

		vkDestroyBuffer(vk_state->device, stagingBuffer, vk_state->allocator);
		vkFreeMemory(vk_state->device, stagingMemory, vk_state->allocator);

		return clientBuffer;
	}

	void DestroyVertexBuffer(VertexBuffer* clientBuffer)
	{
		VulkanVertexBuffer* buffer = (VulkanVertexBuffer*)clientBuffer->internalState;

		vkDestroyBuffer(vk_state->device, buffer->handle, vk_state->allocator);
		vkFreeMemory(vk_state->device, buffer->memory, vk_state->allocator);

		GRFree(clientBuffer);
	}

	IndexBuffer* CreateIndexBuffer(u32* indices, size_t indexCount)
	{
		IndexBuffer* clientBuffer = (IndexBuffer*)GRAlloc(sizeof(IndexBuffer) + sizeof(VulkanIndexBuffer), MEM_TAG_RENDERER_SUBSYS);
		clientBuffer->internalState = clientBuffer + 1;
		VulkanIndexBuffer* buffer = (VulkanIndexBuffer*)clientBuffer->internalState;
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
		stagingAllocateInfo.memoryTypeIndex = FindMemoryType(vk_state->physicalDevice, stagingMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (VK_SUCCESS != vkAllocateMemory(vk_state->device, &stagingAllocateInfo, vk_state->allocator, &stagingMemory))
			GRASSERT(false);

		vkBindBufferMemory(vk_state->device, stagingBuffer, stagingMemory, 0);

		// ================= copying data into staging buffer ===============================
		void* data;
		vkMapMemory(vk_state->device, stagingMemory, 0, stagingBufferCreateInfo.size, 0, &data);
		MemCopy(data, indices, (size_t)stagingBufferCreateInfo.size);
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
		allocateInfo.memoryTypeIndex = FindMemoryType(vk_state->physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (VK_SUCCESS != vkAllocateMemory(vk_state->device, &allocateInfo, vk_state->allocator, &buffer->memory))
			GRASSERT(false);

		vkBindBufferMemory(vk_state->device, buffer->handle, buffer->memory, 0);

		CopyBuffer(buffer->handle, stagingBuffer, bufferCreateInfo.size);

		vkDestroyBuffer(vk_state->device, stagingBuffer, vk_state->allocator);
		vkFreeMemory(vk_state->device, stagingMemory, vk_state->allocator);

		return clientBuffer;
	}

	void DestroyIndexBuffer(IndexBuffer* clientBuffer)
	{
		VulkanIndexBuffer* buffer = (VulkanIndexBuffer*)clientBuffer->internalState;

		vkDestroyBuffer(vk_state->device, buffer->handle, vk_state->allocator);
		vkFreeMemory(vk_state->device, buffer->memory, vk_state->allocator);

		GRFree(clientBuffer);
	}
}
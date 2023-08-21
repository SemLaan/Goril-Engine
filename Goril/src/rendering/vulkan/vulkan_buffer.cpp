#include "../buffer.h"
#include "vulkan_types.h"

namespace GR
{

	u32 FindMemoryType(VkPhysicalDevice physicalDevice, u32 typeFilter, VkMemoryPropertyFlags requiredFlags)
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

	VertexBuffer* CreateVertexBuffer()
	{
		VertexBuffer* clientBuffer = (VertexBuffer*)GRAlloc(sizeof(VertexBuffer), MEM_TAG_RENDERER_SUBSYS);
		clientBuffer->internalState = (VulkanVertexBuffer*)GRAlloc(sizeof(VulkanVertexBuffer), MEM_TAG_RENDERER_SUBSYS);
		VulkanVertexBuffer* buffer = (VulkanVertexBuffer*)clientBuffer->internalState;
		buffer->vertices.Initialize();
		buffer->vertices.Pushback({ {0.0f, -0.5f}, {1.f, 0.f, 0.f} });
		buffer->vertices.Pushback({ {0.5f, 0.5f}, {0.f, 1.f, 0.f} });
		buffer->vertices.Pushback({ {-0.5f, 0.5f}, {0.f, 0.f, 1.f} });

		// ================ Staging buffer =========================
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo stagingBufferCreateInfo = {};
		stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferCreateInfo.pNext = nullptr;
		stagingBufferCreateInfo.flags = 0;
		stagingBufferCreateInfo.size = sizeof(Vertex) * buffer->vertices.Size();
		stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		stagingBufferCreateInfo.queueFamilyIndexCount = 0;
		stagingBufferCreateInfo.pQueueFamilyIndices = nullptr;

		if (VK_SUCCESS != vkCreateBuffer(vk_state->device, &stagingBufferCreateInfo, vk_state->allocator, &stagingBuffer))
			GRASSERT(false);

		VkMemoryRequirements stagingMemoryRequirements;
		vkGetBufferMemoryRequirements(vk_state->device, stagingBuffer, &stagingMemoryRequirements);
		
		VkMemoryAllocateInfo stagingAllocateInfo = {};
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
		MemCopy(data, buffer->vertices.GetRawElements(), (size_t)stagingBufferCreateInfo.size);
		vkUnmapMemory(vk_state->device, stagingMemory);

		// ================= creating the actual buffer =========================
		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = nullptr;
		bufferCreateInfo.flags = 0;
		bufferCreateInfo.size = sizeof(Vertex) * buffer->vertices.Size();
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT; /// TODO: use memory barriers instead of concurrent
		bufferCreateInfo.queueFamilyIndexCount = 2;
		u32 queueFamilyIndices[2] = { vk_state->queueIndices.graphicsFamily, vk_state->queueIndices.transferFamily };
		bufferCreateInfo.pQueueFamilyIndices = queueFamilyIndices;

		if (VK_SUCCESS != vkCreateBuffer(vk_state->device, &bufferCreateInfo, vk_state->allocator, &buffer->handle))
			GRASSERT(false);

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(vk_state->device, buffer->handle, &memoryRequirements);

		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = FindMemoryType(vk_state->physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (VK_SUCCESS != vkAllocateMemory(vk_state->device, &allocateInfo, vk_state->allocator, &buffer->memory))
			GRASSERT(false);

		vkBindBufferMemory(vk_state->device, buffer->handle, buffer->memory, 0);

		return clientBuffer;
	}

	void DestroyVertexBuffer(VertexBuffer* clientBuffer)
	{
		VulkanVertexBuffer* buffer = (VulkanVertexBuffer*)clientBuffer->internalState;

		vkDestroyBuffer(vk_state->device, buffer->handle, vk_state->allocator);
		vkFreeMemory(vk_state->device, buffer->memory, vk_state->allocator);

		buffer->vertices.Deinitialize();
		GRFree(buffer);
		GRFree(clientBuffer);
	}
}
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

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = nullptr;
		bufferCreateInfo.flags = 0;
		bufferCreateInfo.size = sizeof(Vertex) * buffer->vertices.Size();
		bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		//bufferCreateInfo.queueFamilyIndexCount = 1;
		//bufferCreateInfo.pQueueFamilyIndices = &state->queueIndices.graphicsFamily;

		if (VK_SUCCESS != vkCreateBuffer(vk_state->device, &bufferCreateInfo, vk_state->allocator, &buffer->handle))
			GRASSERT(false);

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(vk_state->device, buffer->handle, &memoryRequirements);
		
		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = FindMemoryType(vk_state->physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (VK_SUCCESS != vkAllocateMemory(vk_state->device, &allocateInfo, vk_state->allocator, &buffer->memory))
			GRASSERT(false);

		vkBindBufferMemory(vk_state->device, buffer->handle, buffer->memory, 0);

		void* data;
		vkMapMemory(vk_state->device, buffer->memory, 0, bufferCreateInfo.size, 0, &data);
		MemCopy(data, buffer->vertices.GetRawElements(), (size_t)bufferCreateInfo.size);
		vkUnmapMemory(vk_state->device, buffer->memory);

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
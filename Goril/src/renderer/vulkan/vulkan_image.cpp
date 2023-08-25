// Implements both!
#include "../texture.h"
#include "vulkan_image.h"

#include "vulkan_types.h"
#include "vulkan_buffer.h"


#define IMAGE_CHANNELS 4


namespace GR
{

	b8 CreateImage(VulkanCreateImageParameters* pCreateParameters, VkImage* pImage, VkDeviceMemory* pMemory)
	{
		u32 queueFamilyIndices[2] = { vk_state->graphicsQueue.index, vk_state->transferQueue.index };

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.flags = 0;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = pCreateParameters->format;
		imageCreateInfo.extent.width = pCreateParameters->width;
		imageCreateInfo.extent.height = pCreateParameters->height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = pCreateParameters->tiling;
		imageCreateInfo.usage = pCreateParameters->usage;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		imageCreateInfo.queueFamilyIndexCount = 2;
		imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		if (VK_SUCCESS != vkCreateImage(vk_state->device, &imageCreateInfo, vk_state->allocator, pImage))
		{
			GRFATAL("Vulkan image (texture) creation failed");
			return false;
		}

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(vk_state->device, *pImage, &memoryRequirements);

		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, pCreateParameters->properties);

		if (VK_SUCCESS != vkAllocateMemory(vk_state->device, &allocateInfo, vk_state->allocator, pMemory))
		{
			GRFATAL("Vulkan image (texture) memory allocation failed");
			return false;
		}

		vkBindImageMemory(vk_state->device, *pImage, *pMemory, 0);

		return true;
	}

	Texture CreateTexture(u32 width, u32 height, void* pixels)
	{
		Texture out_texture{};
		out_texture.internalState = GRAlloc(sizeof(VulkanImage), MEM_TAG_TEXTURE);
		VulkanImage* image = (VulkanImage*)out_texture.internalState;

		size_t size = width * height * IMAGE_CHANNELS;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &stagingBuffer, &stagingBufferMemory);

		void* data;
		vkMapMemory(vk_state->device, stagingBufferMemory, 0, size, 0, &data);
		MemCopy(data, pixels, size);
		vkUnmapMemory(vk_state->device, stagingBufferMemory);

		VulkanCreateImageParameters createImageParameters{};
		createImageParameters.width = width;
		createImageParameters.height = height;
		createImageParameters.format = VK_FORMAT_R8G8B8A8_SRGB;
		createImageParameters.tiling = VK_IMAGE_TILING_OPTIMAL;
		createImageParameters.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		createImageParameters.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		
		CreateImage(&createImageParameters, &image->handle, &image->memory);

		return out_texture;
	}
}
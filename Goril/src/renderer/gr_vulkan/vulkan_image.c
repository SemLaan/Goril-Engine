// Implements both!
#include "../texture.h"
#include "vulkan_image.h"

#include "vulkan_types.h"
#include "vulkan_buffer.h"
#include "vulkan_command_buffer.h"
#include "core/logger.h"
#include "core/asserts.h"



bool CreateImage(VulkanCreateImageParameters* pCreateParameters, VkImage* pImage, VkDeviceMemory* pMemory)
{
	VkImageCreateInfo imageCreateInfo = {};
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
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (VK_SUCCESS != vkCreateImage(vk_state->device, &imageCreateInfo, vk_state->allocator, pImage))
	{
		GRFATAL("Vulkan image (texture) creation failed");
		return false;
	}

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(vk_state->device, *pImage, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo = {};
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

Texture TextureCreate(u32 width, u32 height, void* pixels)
{
	Texture out_texture = {};
	out_texture.internalState = Alloc(vk_state->rendererAllocator, sizeof(VulkanImage), MEM_TAG_TEXTURE);
	VulkanImage* image = (VulkanImage*)out_texture.internalState;

	size_t size = width * height * TEXTURE_CHANNELS;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &stagingBuffer, &stagingBufferMemory);

	void* data;
	vkMapMemory(vk_state->device, stagingBufferMemory, 0, size, 0, &data);
	MemCopy(data, pixels, size);
	vkUnmapMemory(vk_state->device, stagingBufferMemory);

	VulkanCreateImageParameters createImageParameters = {};
	createImageParameters.width = width;
	createImageParameters.height = height;
	createImageParameters.format = VK_FORMAT_R8G8B8A8_SRGB;
	createImageParameters.tiling = VK_IMAGE_TILING_OPTIMAL;
	createImageParameters.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	createImageParameters.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	// Creating the image resource
	CreateImage(&createImageParameters, &image->handle, &image->memory);

	// Transitioning the image for copying, copying the image to gpu, transitioning the image for shader reads, transfering image resource ownership to graphics queue
	{
		CommandBuffer commandBuffer;
		AllocateAndBeginSingleUseCommandBuffer(&vk_state->transferQueue, &commandBuffer);

		// Transitioning the image to transfer dst optimal
		VkImageMemoryBarrier2 transitionBarrier = {};
		transitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		transitionBarrier.pNext = nullptr;
		transitionBarrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
		transitionBarrier.srcAccessMask = 0;
		transitionBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
		transitionBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		transitionBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		transitionBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		transitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		transitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		transitionBarrier.image = image->handle;
		transitionBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		transitionBarrier.subresourceRange.baseMipLevel = 0;
		transitionBarrier.subresourceRange.levelCount = 1;
		transitionBarrier.subresourceRange.baseArrayLayer = 0;
		transitionBarrier.subresourceRange.layerCount = 1;

		VkDependencyInfo transitionDependencyInfo = {};
		transitionDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		transitionDependencyInfo.pNext = nullptr;
		transitionDependencyInfo.dependencyFlags = 0;
		transitionDependencyInfo.memoryBarrierCount = 0;
		transitionDependencyInfo.bufferMemoryBarrierCount = 0;
		transitionDependencyInfo.imageMemoryBarrierCount = 1;
		transitionDependencyInfo.pImageMemoryBarriers = &transitionBarrier;

		vkCmdPipelineBarrier2(commandBuffer.handle, &transitionDependencyInfo);

		// Copying the staging buffer into the gpu local image
		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;
		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageOffset.x = 0;
		copyRegion.imageOffset.y = 0;
		copyRegion.imageOffset.z = 0;
		copyRegion.imageExtent.width = width;
		copyRegion.imageExtent.height = height;
		copyRegion.imageExtent.depth = 1;
		
		vkCmdCopyBufferToImage(commandBuffer.handle, stagingBuffer, image->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		// Creating the buffer memory barrier for the queue family release operation
		// Also transition the image to shader read only optimal
		VkImageMemoryBarrier2 releaseImageInfo = {};
		releaseImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		releaseImageInfo.pNext = nullptr;
		releaseImageInfo.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
		releaseImageInfo.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		releaseImageInfo.dstStageMask = 0;  // IGNORED because it is a queue family release operation
		releaseImageInfo.dstAccessMask = 0; // IGNORED because it is a queue family release operation
		releaseImageInfo.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		releaseImageInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		releaseImageInfo.srcQueueFamilyIndex = vk_state->transferQueue.index;
		releaseImageInfo.dstQueueFamilyIndex = vk_state->graphicsQueue.index;
		releaseImageInfo.image = image->handle;
		releaseImageInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		releaseImageInfo.subresourceRange.baseMipLevel = 0;
		releaseImageInfo.subresourceRange.levelCount = 1;
		releaseImageInfo.subresourceRange.baseArrayLayer = 0;
		releaseImageInfo.subresourceRange.layerCount = 1;

		VkDependencyInfo releaseDependencyInfo = {};
		releaseDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		releaseDependencyInfo.pNext = nullptr;
		releaseDependencyInfo.dependencyFlags = 0;
		releaseDependencyInfo.memoryBarrierCount = 0;
		releaseDependencyInfo.bufferMemoryBarrierCount = 0;
		releaseDependencyInfo.imageMemoryBarrierCount = 1;
		releaseDependencyInfo.pImageMemoryBarriers = &releaseImageInfo;

		vkCmdPipelineBarrier2(commandBuffer.handle, &releaseDependencyInfo);

		// Submitting the semaphore that can let other queues know when this index buffer has been uploaded
		vk_state->imageUploadSemaphore.submitValue++;
		VkSemaphoreSubmitInfo semaphoreSubmitInfo = {};
		semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		semaphoreSubmitInfo.pNext = nullptr;
		semaphoreSubmitInfo.semaphore = vk_state->imageUploadSemaphore.handle;
		semaphoreSubmitInfo.value = vk_state->imageUploadSemaphore.submitValue;
		semaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
		semaphoreSubmitInfo.deviceIndex = 0;

		u64 signaledValue;
		EndSubmitAndFreeSingleUseCommandBuffer(commandBuffer, 0, nullptr, 1, &semaphoreSubmitInfo, &signaledValue);

		// Making sure the staging buffer and memory get deleted when their corresponding command buffer is completed
		ResourceDestructionInfo bufferDestructionInfo = {};
		bufferDestructionInfo.resource = stagingBuffer;
		bufferDestructionInfo.Destructor = VulkanBufferDestructor;
		bufferDestructionInfo.signalValue = signaledValue;

		ResourceDestructionInfo memoryDestructionInfo = {};
		memoryDestructionInfo.resource = stagingBufferMemory;
		memoryDestructionInfo.Destructor = VulkanMemoryDestructor;
		memoryDestructionInfo.signalValue = signaledValue;

		vk_state->transferQueue.resourcesPendingDestructionDarray = (ResourceDestructionInfo*)DarrayPushback(vk_state->transferQueue.resourcesPendingDestructionDarray, &bufferDestructionInfo);
		vk_state->transferQueue.resourcesPendingDestructionDarray = (ResourceDestructionInfo*)DarrayPushback(vk_state->transferQueue.resourcesPendingDestructionDarray, &memoryDestructionInfo);
	}

	// Creating the image memory barrier for the queue family acquire operation
	// This is put in the requestedQueueAcquisitionOperations list and will be submitted as a command in the draw loop, 
	// also synced with image upload semaphore, so ownership isn't acquired before it is released
	VkDependencyInfo* acquireDependencyInfo = Alloc(vk_state->resourceAcquisitionPool, sizeof(VkDependencyInfo) + sizeof(VkImageMemoryBarrier2), MEM_TAG_RENDERER_SUBSYS);
	VkImageMemoryBarrier2* acquireImageInfo = (VkImageMemoryBarrier2*)(acquireDependencyInfo + 1);

	acquireImageInfo->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	acquireImageInfo->pNext = nullptr;
	acquireImageInfo->srcStageMask = 0;  // IGNORED because it is a queue family acquire operation
	acquireImageInfo->srcAccessMask = 0; // IGNORED because it is a queue family acquire operation
	acquireImageInfo->dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
	acquireImageInfo->dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
	acquireImageInfo->oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	acquireImageInfo->newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	acquireImageInfo->srcQueueFamilyIndex = vk_state->transferQueue.index;
	acquireImageInfo->dstQueueFamilyIndex = vk_state->graphicsQueue.index;
	acquireImageInfo->image = image->handle;
	acquireImageInfo->subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	acquireImageInfo->subresourceRange.baseMipLevel = 0;
	acquireImageInfo->subresourceRange.levelCount = 1;
	acquireImageInfo->subresourceRange.baseArrayLayer = 0;
	acquireImageInfo->subresourceRange.layerCount = 1;

	acquireDependencyInfo->sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	acquireDependencyInfo->pNext = nullptr;
	acquireDependencyInfo->dependencyFlags = 0;
	acquireDependencyInfo->memoryBarrierCount = 0;
	acquireDependencyInfo->bufferMemoryBarrierCount = 0;
	acquireDependencyInfo->imageMemoryBarrierCount = 1;
	acquireDependencyInfo->pImageMemoryBarriers = acquireImageInfo;

	vk_state->requestedQueueAcquisitionOperationsDarray = (VkDependencyInfo**)DarrayPushback(vk_state->requestedQueueAcquisitionOperationsDarray, &acquireDependencyInfo);

	// Creating the image view
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.pNext = nullptr;
	viewCreateInfo.flags = 0;
	viewCreateInfo.image = image->handle;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = 1;

	if (VK_SUCCESS != vkCreateImageView(vk_state->device, &viewCreateInfo, vk_state->allocator, &image->view))
	{
		GRFATAL("Texture image view creation failed");
		GRASSERT(false);
	}

	// Creating the sampler
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = nullptr;
	samplerCreateInfo.flags = 0;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.anisotropyEnable = VK_FALSE; /// TODO: allow enabling anisotropy
	samplerCreateInfo.maxAnisotropy = 1.0f;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

	if (VK_SUCCESS != vkCreateSampler(vk_state->device, &samplerCreateInfo, vk_state->allocator, &image->sampler))
	{
		GRASSERT_MSG(false, "failed to create image sampler");
	}

	return out_texture;
}

static void ImageDestructor(void* resource)
{
	VulkanImage* image = (VulkanImage*)resource;

	vkDestroySampler(vk_state->device, image->sampler, vk_state->allocator);
	vkDestroyImageView(vk_state->device, image->view, vk_state->allocator);
	vkDestroyImage(vk_state->device, image->handle, vk_state->allocator);
	vkFreeMemory(vk_state->device, image->memory, vk_state->allocator);

	Free(vk_state->rendererAllocator, image);
}

void TextureDestroy(Texture clientTexture)
{
	ResourceDestructionInfo imageDestructionInfo = {};
	imageDestructionInfo.resource = clientTexture.internalState;
	imageDestructionInfo.Destructor = ImageDestructor;
	imageDestructionInfo.signalValue = vk_state->graphicsQueue.semaphore.submitValue;

	vk_state->graphicsQueue.resourcesPendingDestructionDarray = (ResourceDestructionInfo*)DarrayPushback(vk_state->graphicsQueue.resourcesPendingDestructionDarray, &imageDestructionInfo);
}

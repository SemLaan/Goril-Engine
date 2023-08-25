#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"

namespace GR
{
	b8 CreateBuffer(VkDeviceSize size, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkBuffer* out_buffer, VkDeviceMemory* out_memory);

	/// TODO: make custom vulkan allocator and move this there
	u32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags requiredFlags);
}
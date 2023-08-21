#pragma once
#include <vulkan/vulkan.h>
#include "defines.h"
#include "containers/darray.h"
#include "../buffer.h"

namespace GR
{
	struct RendererState;
	extern RendererState* vk_state;

	struct VulkanVertexBuffer
	{
		VkDeviceSize size;
		VkBuffer handle;
		VkDeviceMemory memory;
	};

	struct VulkanIndexBuffer
	{
		VkDeviceSize size;
		VkBuffer handle;
		VkDeviceMemory memory;
		size_t indexCount;
	};

	struct SwapchainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		Darray<VkSurfaceFormatKHR> formats;
		Darray<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices
	{
		u32 graphicsFamily;
		//u32 computeFamily;
		u32 presentFamily;
		u32 transferFamily;
	};

	struct RendererState
	{
		VkInstance instance;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkQueue transferQueue;
		QueueFamilyIndices queueIndices;
		VkSurfaceKHR surface;
		SwapchainSupportDetails swapchainSupport;
		VkSwapchainKHR swapchain;
		Darray<VkImage> swapchainImages;
		Darray<VkImageView> swapchainImageViews;
		VkFormat swapchainFormat;
		VkExtent2D swapchainExtent;
		VkRenderPass renderpass;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		Darray<VkFramebuffer> swapchainFramebuffers;
		VkCommandPool graphicsCommandPool;
		VkCommandPool transferCommandPool;
		Darray<VkCommandBuffer> commandBuffers;
		Darray<VkSemaphore> imageAvailableSemaphores;
		Darray<VkSemaphore> renderFinishedSemaphores;
		Darray<VkFence> inFlightFences;
		i32 maxFramesInFlight;
		u32 currentFrame;
		b8 shouldRecreateSwapchain;
		VertexBuffer* vertexBuffer;
		IndexBuffer* indexBuffer;
		VkAllocationCallbacks* allocator;
#ifndef GR_DIST
		VkDebugUtilsMessengerEXT debugMessenger;
#endif // !GR_DIST
	};
}
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

	struct VulkanImage
	{
		VkImage handle;
		VkDeviceMemory memory;
	};

	struct VulkanSemaphore
	{
		VkSemaphore handle;
		u64 submitValue;
	};

	struct QueueFamily
	{
		VkQueue handle;
		VkCommandPool commandPool;
		u32 index;
	};

	struct CommandBuffer
	{
		VkCommandBuffer handle;
		QueueFamily* queueFamily;
	};

	struct SwapchainSupportDetails 
	{
		VkSurfaceCapabilitiesKHR capabilities;
		Darray<VkSurfaceFormatKHR> formats;
		Darray<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices
	{
		u32 graphicsFamily;
		u32 presentFamily;
		u32 transferFamily;
	};

	typedef void (*PFN_ResourceDestructor)(void* resource);

	struct InFlightTemporaryResource
	{
		void* resource;
		PFN_ResourceDestructor Destructor;
		u64 signalValue;
	};

	struct RendererState
	{
		VkInstance instance;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		QueueFamily graphicsQueue;
		QueueFamily transferQueue;
		VkQueue presentQueue;
		QueueFamilyIndices queueIndices;
		VkSurfaceKHR surface;
		SwapchainSupportDetails swapchainSupport;
		VkSwapchainKHR swapchain;
		Darray<VkImage> swapchainImages;
		Darray<VkImageView> swapchainImageViews;
		VkFormat swapchainFormat;
		VkExtent2D swapchainExtent;
		VkRenderPass renderpass;
		VkDescriptorSetLayout descriptorSetLayout;
		Darray<VkBuffer> uniformBuffers;
		Darray<VkDeviceMemory> uniformBuffersMemory;
		Darray<void*> uniformBuffersMapped;
		VkDescriptorPool uniformDescriptorPool;
		Darray<VkDescriptorSet> uniformDescriptorSets;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		Darray<VkFramebuffer> swapchainFramebuffers;
		Darray<CommandBuffer*> commandBuffers;
		Darray<VkSemaphore> imageAvailableSemaphores;
		Darray<VkSemaphore> renderFinishedSemaphores;
		Darray<VkFence> inFlightFences;
		VulkanSemaphore vertexUploadSemaphore;
		VulkanSemaphore indexUploadSemaphore;
		VulkanSemaphore imageUploadSemaphore;
		VulkanSemaphore singleUseCommandBufferSemaphore;
		Darray<InFlightTemporaryResource> singleUseCommandBufferResourcesInFlight;
		Darray<VkDependencyInfo*> requestedQueueAcquisitionOperations;
		i32 maxFramesInFlight;
		u32 currentFrame;
		u32 currentSwapchainImageIndex;
		b8 shouldRecreateSwapchain;
		VkAllocationCallbacks* allocator;
#ifndef GR_DIST
		VkDebugUtilsMessengerEXT debugMessenger;
#endif // !GR_DIST
	};
}
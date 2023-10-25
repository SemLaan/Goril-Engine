#pragma once
#include <vulkan/vulkan.h>
#include "defines.h"
#include "containers/darray.h"
#include "../buffer.h"

typedef struct RendererState RendererState;
extern RendererState* vk_state;

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct VulkanVertexBuffer
{
	VkDeviceSize size;
	VkBuffer handle;
	VkDeviceMemory memory;
} VulkanVertexBuffer;

typedef struct VulkanIndexBuffer
{
	VkDeviceSize size;
	VkBuffer handle;
	VkDeviceMemory memory;
	size_t indexCount;
} VulkanIndexBuffer;

typedef struct VulkanImage
{
	VkImage handle;
	VkImageView view;
	VkSampler sampler;
	VkDeviceMemory memory;
} VulkanImage;

typedef struct VulkanSemaphore
{
	VkSemaphore handle;
	u64 submitValue;
} VulkanSemaphore;

typedef void (*PFN_ResourceDestructor)(void* resource);

typedef struct ResourceDestructionInfo
{
	void* resource;
	PFN_ResourceDestructor	Destructor;
	u64						signalValue;
} ResourceDestructionInfo;

typedef struct QueueFamily
{
	VkQueue handle;
	VkCommandPool commandPool;
	ResourceDestructionInfo* resourcesPendingDestructionDarray;
	VulkanSemaphore semaphore;
	u32 index;
} QueueFamily;

typedef struct CommandBuffer
{
	VkCommandBuffer handle;
	QueueFamily* queueFamily;
} CommandBuffer;


typedef struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR* formatsDarray;
	VkPresentModeKHR* presentModesDarray;
} SwapchainSupportDetails;

typedef struct RendererState
{
	// Frequently used data (every frame)
	VkDevice device;
	VkSwapchainKHR swapchain;
	VkQueue presentQueue;
	VkFramebuffer* swapchainFramebuffersDarray;	// TODO: gets removed when i switch to dynamic renderpasses
	CommandBuffer* graphicsCommandBuffers[MAX_FRAMES_IN_FLIGHT]; //TODO: ideally is not a pointer
	u32 currentInFlightFrameIndex;
	u32 currentSwapchainImageIndex;
	bool shouldRecreateSwapchain;
	VkExtent2D swapchainExtent;

	// Binary semaphores for synchronizing the swapchain with the screen and the GPU
	VkSemaphore* imageAvailableSemaphoresDarray;	//TODO: change to static array to remove pointer indirection
	VkSemaphore* renderFinishedSemaphoresDarray;	//TODO: change to static array to remove pointer indirection

	// Timeline semaphores for synchronizing uploads and 
	VulkanSemaphore vertexUploadSemaphore;
	VulkanSemaphore indexUploadSemaphore;
	VulkanSemaphore imageUploadSemaphore;
	VulkanSemaphore frameSemaphore;

	// TODO: state that needs to be moved out of the global renderer state
	VkRenderPass renderpass;// TODO: factor into 2d renderer and switch to dynamic renderpass
	VkDescriptorSetLayout descriptorSetLayout;// TODO: factor into 2d renderer
	VkBuffer* uniformBuffersDarray;// TODO: factor into 2d renderer
	VkDeviceMemory* uniformBuffersMemoryDarray;// TODO: factor into 2d renderer
	void** uniformBuffersMappedDarray;// TODO: factor into 2d renderer
	VkDescriptorPool uniformDescriptorPool;// TODO: factor into 2d renderer
	VkDescriptorSet* uniformDescriptorSetsDarray; // TODO: factor into 2d renderer 
	VkPipelineLayout pipelineLayout;	// TODO: factor into 2d renderer
	VkPipeline graphicsPipeline;		// TODO: factor into 2d renderer

	// Data that is not used every frame or possibly used every frame
	QueueFamily graphicsQueue;
	QueueFamily transferQueue;
	VkDependencyInfo** requestedQueueAcquisitionOperationsDarray;
	VkAllocationCallbacks* allocator;

	// Data that is only used on startup/shutdown
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	SwapchainSupportDetails swapchainSupport;
	u32 presentQueueFamilyIndex;
	VkSurfaceKHR surface; // TODO: check where this is used
	VkFormat swapchainFormat;
	VkImage* swapchainImagesDarray; // TODO: this might become used often once the switch to dynamic renderpasses is made
	VkImageView* swapchainImageViewsDarray; // TODO: this might become used often once the switch to dynamic renderpasses is made
#ifndef GR_DIST
	VkDebugUtilsMessengerEXT debugMessenger;
#endif // !GR_DIST
} RendererState;

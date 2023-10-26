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
	VkDevice device;												// Logical device
	VkSwapchainKHR swapchain;										// Swapchain handle
	VkQueue presentQueue;											// Present queue handle
	VkFramebuffer* swapchainFramebuffersDarray;						// TODO: gets removed when i switch to dynamic renderpasses
	CommandBuffer graphicsCommandBuffers[MAX_FRAMES_IN_FLIGHT]; 	// Command buffers for recording entire frames to the graphics queue
	u32 currentInFlightFrameIndex;									// Current frame % MAX_FRAMES_IN_FLIGHT
	u32 currentSwapchainImageIndex;									// Current swapchain image index (current frame % swapchain image count)
	bool shouldRecreateSwapchain;									// Checked at the start of each renderloop, is set to true upon window resize
	VkExtent2D swapchainExtent;										// Extent of the swapchain, used for beginning renderpass TODO: make the renderpass info beforehand instead of constructing it 
																	// every frame (actually doing it every frame might be faster, because the space being used on the stack is already in cache)

	// Binary semaphores for synchronizing the swapchain with the screen and the GPU
	VkSemaphore* imageAvailableSemaphoresDarray;					// Binary semaphores that synchronize swapchain image acquisition TODO: change to static array to remove pointer indirection
	VkSemaphore* renderFinishedSemaphoresDarray;					// Binary semaphores that synchronize swapchain image presentation TODO: change to static array to remove pointer indirection

	// Timeline semaphores for synchronizing uploads and 
	VulkanSemaphore vertexUploadSemaphore;							// Timeline semaphore that synchronizes vertex upload with vertex input
	VulkanSemaphore indexUploadSemaphore;							// Timeline semaphore that synchronizes index upload with index input
	VulkanSemaphore imageUploadSemaphore;							// Timeline semaphore that synchronizes image upload with image usage
	VulkanSemaphore frameSemaphore;									// Timeline semaphore that synchronizes rendering resources

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
	VkSurfaceKHR surface;
	VkFormat swapchainFormat;
	VkImage* swapchainImagesDarray; // TODO: this might become used often once the switch to dynamic renderpasses is made
	VkImageView* swapchainImageViewsDarray; // TODO: this might become used often once the switch to dynamic renderpasses is made
#ifndef GR_DIST
	VkDebugUtilsMessengerEXT debugMessenger;
#endif // !GR_DIST
} RendererState;

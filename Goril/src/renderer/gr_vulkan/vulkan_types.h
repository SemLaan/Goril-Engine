#pragma once
#include <vulkan/vulkan.h>
#include "defines.h"
#include "containers/darray.h"
#include "../buffer.h"

typedef struct RendererState RendererState;
extern RendererState* vk_state;

#define MAX_FRAMES_IN_FLIGHT 2
#define RENDER_POOL_BLOCK_SIZE_32 32
#define QUEUE_ACQUISITION_POOL_BLOCK_SIZE 160 // 160 bytes (2.5 cache lines) 32 byte aligned, enough to store VkDependencyInfo + (VkImageMemoryBarrier2 or VkBufferMemoryBarrier2)


// TODO: if still under 32B when finished, switch to pool allocator (or some other size pool allocator)
typedef struct VulkanVertexBuffer
{
	VkDeviceSize size;
	VkBuffer handle;
	VkDeviceMemory memory;
} VulkanVertexBuffer;

// TODO: if still under 32B when finished, switch to pool allocator (or some other size pool allocator)
typedef struct VulkanIndexBuffer
{
	VkDeviceSize size;
	VkBuffer handle;
	VkDeviceMemory memory;
	size_t indexCount;
} VulkanIndexBuffer;

// TODO: if still under 32B when finished, switch to pool allocator (or some other size pool allocator)
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
	u32 formatCount;
	VkSurfaceFormatKHR* formats;
	u32 presentModeCount;
	VkPresentModeKHR* presentModes;
} SwapchainSupportDetails;

typedef struct RendererState
{
	// Frequently used data (every frame)
	VkDevice device;												// Logical device
	VkSwapchainKHR swapchain;										// Swapchain handle
	VkQueue presentQueue;											// Present queue handle
	CommandBuffer graphicsCommandBuffers[MAX_FRAMES_IN_FLIGHT]; 	// Command buffers for recording entire frames to the graphics queue
	u32 currentInFlightFrameIndex;									// Current frame % MAX_FRAMES_IN_FLIGHT
	u32 currentSwapchainImageIndex;									// Current swapchain image index (current frame % swapchain image count)
	VkImage* swapchainImages;										// Images that make up the swapchain (retrieved from the swapchain)
	VkImageView* swapchainImageViews;								// Image views that make up the swapchain (created from swapchain images)
	bool shouldRecreateSwapchain;									// Checked at the start of each renderloop, is set to true upon window resize
	VkExtent2D swapchainExtent;										// Extent of the swapchain, used for beginning renderpass

	// Binary semaphores for synchronizing the swapchain with the screen and the GPU
	VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];		// Binary semaphores that synchronize swapchain image acquisition TODO: change to timeline semaphore once vulkan allows it (hopefully 1.4)
	VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];		// Binary semaphores that synchronize swapchain image presentation TODO: change to timeline semaphore once vulkan allows it (hopefully 1.4)

	// Timeline semaphores for synchronizing uploads and 
	VulkanSemaphore vertexUploadSemaphore;							// Timeline semaphore that synchronizes vertex upload with vertex input
	VulkanSemaphore indexUploadSemaphore;							// Timeline semaphore that synchronizes index upload with index input
	VulkanSemaphore imageUploadSemaphore;							// Timeline semaphore that synchronizes image upload with image usage
	VulkanSemaphore frameSemaphore;									// Timeline semaphore that synchronizes rendering resources

	// Allocators
	Allocator* rendererAllocator;									// Global allocator of the renderer subsys
	Allocator* rendererBumpAllocator;								// Bump allocator for the renderer subsys
	Allocator* poolAllocator32B;									// Pool allocator of the renderer subsys
	Allocator* resourceAcquisitionPool;								// Pool allocator for resource acquisition operation infos (memory barriers)

	// Data that is not used every frame or possibly used every frame
	QueueFamily graphicsQueue;										// Graphics family queue
	QueueFamily transferQueue;										// Transfer family queue
	VkDependencyInfo** requestedQueueAcquisitionOperationsDarray;	// For transfering queue ownership from transfer to graphics after a resource has been uploaded
	VkAllocationCallbacks* vkAllocator;								// Vulkan API allocator, only for reading vulkan allocations not for taking over allocation from vulkan //TODO: this is currently just nullptr

	// Data that is only used on startup/shutdown
	VkInstance instance;											// Vulkan instance handle
	VkPhysicalDevice physicalDevice;								// Physical device handle
	SwapchainSupportDetails swapchainSupport;						// Data about swapchain capabilities
	u32 presentQueueFamilyIndex;									// What it says on the tin
	VkSurfaceKHR surface;											// Vulkan surface handle
	VkFormat swapchainFormat;										// Format of the swapchain
	u32 swapchainImageCount;										// Amount of images in the swapchain
	Texture defaultTexture;											// Default texture
#ifndef GR_DIST
	VkDebugUtilsMessengerEXT debugMessenger;						// Debug messenger, only exists in debug mode
#endif // !GR_DIST
} RendererState;

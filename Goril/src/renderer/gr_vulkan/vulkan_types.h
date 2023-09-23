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

typedef struct QueueFamilyIndices
{
	u32 graphicsFamily;
	u32 presentFamily;
	u32 transferFamily;
} QueueFamilyIndices;

//TODO: refactor this mess
typedef struct RendererState
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
	VkImage* swapchainImagesDarray;
	VkImageView* swapchainImageViewsDarray;
	VkFormat swapchainFormat;
	VkExtent2D swapchainExtent;
	VkRenderPass renderpass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkBuffer* uniformBuffersDarray;
	VkDeviceMemory* uniformBuffersMemoryDarray;
	void** uniformBuffersMappedDarray;
	VkDescriptorPool uniformDescriptorPool;
	VkDescriptorSet* uniformDescriptorSetsDarray;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkFramebuffer* swapchainFramebuffersDarray;
	CommandBuffer* commandBuffers[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore* imageAvailableSemaphoresDarray;
	VkSemaphore* renderFinishedSemaphoresDarray;
	VulkanSemaphore vertexUploadSemaphore;
	VulkanSemaphore indexUploadSemaphore;
	VulkanSemaphore imageUploadSemaphore;
	VulkanSemaphore frameSemaphore;
	VkDependencyInfo** requestedQueueAcquisitionOperationsDarray;
	u32 currentFrame;
	u32 currentSwapchainImageIndex;
	bool shouldRecreateSwapchain;
	VkAllocationCallbacks* allocator;
#ifndef GR_DIST
	VkDebugUtilsMessengerEXT debugMessenger;
#endif // !GR_DIST
} RendererState;
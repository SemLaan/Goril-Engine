#include "../renderer.h"

#include "core/logger.h"
#include "core/asserts.h"
#include "core/gr_memory.h"
#include "core/event.h"
#include "containers/darray.h"

#include "vulkan_platform.h"
#include "vulkan_types.h"
#include "vulkan_debug_messenger.h"
#include "vulkan_instance.h"
#include "vulkan_device.h"
#include "vulkan_queues.h"
#include "vulkan_swapchain.h"
#include "vulkan_graphics_pipeline.h"
#include "vulkan_renderpass.h"
#include "vulkan_command_buffer.h"
#include "vulkan_sync_objects.h"
#include "../buffer.h"


RendererState* vk_state = nullptr;

static bool OnWindowResize(EventCode type, EventData data);

bool InitializeRenderer()
{
	GRASSERT_DEBUG(vk_state == nullptr); // If this triggers init got called twice
	GRINFO("Initializing renderer subsystem...");

	vk_state = (RendererState*)Alloc(GetGlobalAllocator(), sizeof(RendererState), MEM_TAG_RENDERER_SUBSYS);
	vk_state->allocator = nullptr;

	vk_state->currentFrame = 0;
	vk_state->shouldRecreateSwapchain = false;

	RegisterEventListener(EVCODE_WINDOW_RESIZED, OnWindowResize);

	// ================== Getting required extensions and layers ================================
	// Getting required extensions
	void** requiredExtensionsDarray = (void**)DarrayCreate(sizeof(void*), 5, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS); // TODO: Change allocator
	GetPlatformExtensions(requiredExtensionsDarray);
	const char* vk_khr_surface_extension_name = VK_KHR_SURFACE_EXTENSION_NAME;
	requiredExtensionsDarray = (void**)DarrayPushback(requiredExtensionsDarray, &vk_khr_surface_extension_name);
#ifndef GR_DIST
	const char* vk_ext_debug_utils_extension_name = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	requiredExtensionsDarray = (void**)DarrayPushback(requiredExtensionsDarray, &vk_ext_debug_utils_extension_name);
#endif // !GR_DIST

	// Getting required layers
	void** requiredLayersDarray = (void**)DarrayCreate(sizeof(void*), 1, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS); // TODO: Change allocator
#ifndef GR_DIST
	const char* vk_layer_khronos_validation = "VK_LAYER_KHRONOS_validation";
	requiredLayersDarray = (void**)DarrayPushback(requiredLayersDarray, &vk_layer_khronos_validation);
#endif // !GR_DIST


	// ================== Creating instance =================================
	if (!CreateVulkanInstance(requiredExtensionsDarray, requiredLayersDarray))
	{
		DarrayDestroy(requiredExtensionsDarray);
		DarrayDestroy(requiredLayersDarray);
		return false;
	}
	DarrayDestroy(requiredExtensionsDarray);

	// =============== Creating debug messenger ============================
#ifndef GR_DIST
	if (!CreateDebugMessenger())
	{
		DarrayDestroy(requiredLayersDarray);
		return false;
	}
#endif // !GR_DIST

	// ================ Creating a surface =====================================
	if (!PlatformCreateSurface(vk_state->instance, vk_state->allocator, &vk_state->surface))
	{
		GRFATAL("Failed to create Vulkan surface");
		DarrayDestroy(requiredLayersDarray);
		return false;
	}

	// ================ Getting a physical device ==============================
	void** requiredDeviceExtensionsDarray = (void**)DarrayCreate(sizeof(void*), 3, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS);
	const char* vk_khr_swapchain_extension_name = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	const char* vk_khr_synch2_extension_name = VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME;
	const char* vk_khr_timeline_semaphore_extension_name = VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME;
	requiredDeviceExtensionsDarray = (void**)DarrayPushback(requiredDeviceExtensionsDarray, &vk_khr_swapchain_extension_name);
	requiredDeviceExtensionsDarray = (void**)DarrayPushback(requiredDeviceExtensionsDarray, &vk_khr_synch2_extension_name);
	requiredDeviceExtensionsDarray = (void**)DarrayPushback(requiredDeviceExtensionsDarray, &vk_khr_timeline_semaphore_extension_name);

	if (!SelectPhysicalDevice(requiredDeviceExtensionsDarray))
	{
		DarrayDestroy(requiredLayersDarray);
		return false;
	}

	// ================== Getting device queue families ==============================
	SelectQueueFamilies(vk_state);

	// ===================== Creating logical device =============================================
	if (!CreateLogicalDevice(vk_state, requiredDeviceExtensionsDarray, requiredLayersDarray))
	{
		DarrayDestroy(requiredLayersDarray);
		return false;
	}
	DarrayDestroy(requiredLayersDarray);

	// ===================== sets up queues and command pools ================================
	if (!CreateQueues())
		return false;

	// ======================== Creating the swapchain ===============================================
	if (!CreateSwapchain(vk_state))
		return false;

	// ========================== Creating renderpass ==============================================
	if (!CreateRenderpass(vk_state))
		return false;

	// ======================== Creating graphics pipeline ============================================
	if (!CreateGraphicsPipeline())
		return false;

	// ======================= Create swapchain framebuffers ============================================
	if (!CreateSwapchainFramebuffers(vk_state))
		return false;

	// ============================ Allocate a command buffer =======================================
	for (i32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (!AllocateCommandBuffer(&vk_state->graphicsQueue, &vk_state->commandBuffers[i]))
			return false;
	}

	// ================================ Create sync objects ===========================================
	if (!CreateSyncObjects())
		return false;

	vk_state->requestedQueueAcquisitionOperationsDarray = (VkDependencyInfo**)DarrayCreate(sizeof(VkDependencyInfo*), 10, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS); /// TODO: change allocator to renderer local allocator (when it exists)

	return true;
}

void ShutdownRenderer()
{
	if (vk_state == nullptr)
	{
		GRINFO("Renderer startup failed, skipping shutdown");
		return;
	}
	else
	{
		GRINFO("Shutting down renderer subsystem...");
	}

	UnregisterEventListener(EVCODE_WINDOW_RESIZED, OnWindowResize);

	if (vk_state->device)
		vkDeviceWaitIdle(vk_state->device);

	if (vk_state->graphicsQueue.resourcesPendingDestructionDarray)
		TryDestroyResourcesPendingDestruction();

	if (vk_state->requestedQueueAcquisitionOperationsDarray)
		DarrayDestroy(vk_state->requestedQueueAcquisitionOperationsDarray);

	// ================================ Destroy sync objects if they were created ===========================================
	DestroySyncObjects();

	// ====================== Destroying swapchain framebuffers if they were created ================================
	DestroySwapchainFramebuffers(vk_state);

	// ====================== Destroying graphics pipeline if it was created ================================
	DestroyGraphicsPipeline();

	// ======================== Destroying renderpass if it was created =====================================
	DestroyRenderpass(vk_state);

	// ====================== Destroying swapchain if it was created ================================
	DestroySwapchain(vk_state);

	// ===================== destroys queues and command pools ================================
	DestroyQueues();

	// ===================== Destroying logical device if it was created =================================
	DestroyLogicalDevice(vk_state);

	// ======================= Destroying the surface if it was created ==================================
	if (vk_state->surface)
		vkDestroySurfaceKHR(vk_state->instance, vk_state->surface, vk_state->allocator);

	// ===================== Destroying debug messenger if it was created =================================
#ifndef GR_DIST
	DestroyDebugMessenger();
#endif // !GR_DIST

	// ======================= Destroying instance if it was created =======================================
	DestroyVulkanInstance();
	Free(GetGlobalAllocator(), vk_state);
	vk_state = nullptr;
}

void RecreateSwapchain()
{
	vkDeviceWaitIdle(vk_state->device);

	DestroySwapchainFramebuffers(vk_state);
	DestroySwapchain(vk_state);

	CreateSwapchain(vk_state);
	CreateSwapchainFramebuffers(vk_state);

	vk_state->shouldRecreateSwapchain = false;
	GRINFO("Vulkan Swapchain resized");
}

bool BeginFrame()
{
	TryDestroyResourcesPendingDestruction();

	if (vk_state->shouldRecreateSwapchain)
		RecreateSwapchain();

	VkSemaphoreWaitInfo semaphoreWaitInfo = {};
	semaphoreWaitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	semaphoreWaitInfo.pNext = nullptr;
	semaphoreWaitInfo.flags = 0;
	semaphoreWaitInfo.semaphoreCount = 1;
	semaphoreWaitInfo.pSemaphores = &vk_state->frameSemaphore.handle;
	u64 waitForValue = vk_state->frameSemaphore.submitValue - (MAX_FRAMES_IN_FLIGHT - 1);
	semaphoreWaitInfo.pValues = &waitForValue;

	vkWaitSemaphores(vk_state->device, &semaphoreWaitInfo, UINT64_MAX);

	VkResult result = vkAcquireNextImageKHR(vk_state->device, vk_state->swapchain, UINT64_MAX, vk_state->imageAvailableSemaphoresDarray[vk_state->currentFrame], VK_NULL_HANDLE, &vk_state->currentSwapchainImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		vk_state->shouldRecreateSwapchain = true;
		return false;
	}
	else if (result == VK_SUBOPTIMAL_KHR)
	{
		// Sets recreate swapchain to true BUT DOES NOT RETURN because the image has been acquired so we can continue rendering for this frame
		vk_state->shouldRecreateSwapchain = true;
	}
	else if (result != VK_SUCCESS)
	{
		GRWARN("Failed to acquire next swapchain image");
		return false;
	}

	// ===================================== Begin command buffer recording =========================================
	ResetAndBeginCommandBuffer(vk_state->commandBuffers[vk_state->currentFrame]);
	VkCommandBuffer currentCommandBuffer = vk_state->commandBuffers[vk_state->currentFrame]->handle;

	// acquire ownership of all uploaded resources
	for (u32 i = 0; i < DarrayGetSize(vk_state->requestedQueueAcquisitionOperationsDarray); ++i)
	{
		vkCmdPipelineBarrier2(currentCommandBuffer, vk_state->requestedQueueAcquisitionOperationsDarray[i]);
		Free(GetGlobalAllocator(), vk_state->requestedQueueAcquisitionOperationsDarray[i]);
	}

	DarraySetSize(vk_state->requestedQueueAcquisitionOperationsDarray, 0);

	/// TODO: begin renderpass function
	VkClearValue clearColor = {};
	clearColor.color.float32[0] = 0;
	clearColor.color.float32[1] = 0;
	clearColor.color.float32[2] = 0;
	clearColor.color.float32[3] = 1.0f;
	VkRenderPassBeginInfo renderpassBeginInfo = {};
	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassBeginInfo.pNext = nullptr;
	renderpassBeginInfo.renderPass = vk_state->renderpass;
	renderpassBeginInfo.framebuffer = vk_state->swapchainFramebuffersDarray[vk_state->currentSwapchainImageIndex];
	renderpassBeginInfo.renderArea.offset.x = 0;
	renderpassBeginInfo.renderArea.offset.y = 0;
	renderpassBeginInfo.renderArea.extent = vk_state->swapchainExtent;
	renderpassBeginInfo.clearValueCount = 1;
	renderpassBeginInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(currentCommandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	/// TODO: bind graphics pipeline function
	vkCmdBindPipeline(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_state->graphicsPipeline);

	// Viewport and scissor
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = (f32)vk_state->swapchainExtent.height;
	viewport.width = (f32)vk_state->swapchainExtent.width;
	viewport.height = -(f32)vk_state->swapchainExtent.height;
	viewport.minDepth = 1.0f;
	viewport.maxDepth = 0.0f;
	vkCmdSetViewport(currentCommandBuffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = vk_state->swapchainExtent;
	vkCmdSetScissor(currentCommandBuffer, 0, 1, &scissor);

	vkCmdBindDescriptorSets(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_state->pipelineLayout, 0, 1, &vk_state->uniformDescriptorSetsDarray[vk_state->currentFrame], 0, nullptr);

	return true;
}

void EndFrame()
{
	/// TODO: end renderpass function
	vkCmdEndRenderPass(vk_state->commandBuffers[vk_state->currentFrame]->handle);

	// ==================== End command buffer recording ==================================================
	EndCommandBuffer(vk_state->commandBuffers[vk_state->currentFrame]);

	// Submitting command buffer
	#define WAIT_SEMAPHORE_COUNT 4 // 1 swapchain image acquisition, 3 resourse upload waits
	VkSemaphoreSubmitInfo waitSemaphores[WAIT_SEMAPHORE_COUNT] = {};

	// Swapchain image acquisition semaphore
	waitSemaphores[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	waitSemaphores[0].pNext = nullptr;
	waitSemaphores[0].semaphore = vk_state->imageAvailableSemaphoresDarray[vk_state->currentFrame];
	waitSemaphores[0].value = 0;
	waitSemaphores[0].stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	waitSemaphores[0].deviceIndex = 0;

	// Resource upload semaphores
	waitSemaphores[1].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	waitSemaphores[1].pNext = nullptr;
	waitSemaphores[1].semaphore = vk_state->vertexUploadSemaphore.handle;
	waitSemaphores[1].value = vk_state->vertexUploadSemaphore.submitValue;
	waitSemaphores[1].stageMask = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
	waitSemaphores[1].deviceIndex = 0;

	waitSemaphores[2].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	waitSemaphores[2].pNext = nullptr;
	waitSemaphores[2].semaphore = vk_state->indexUploadSemaphore.handle;
	waitSemaphores[2].value = vk_state->indexUploadSemaphore.submitValue;
	waitSemaphores[2].stageMask = VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
	waitSemaphores[2].deviceIndex = 0;

	waitSemaphores[3].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	waitSemaphores[3].pNext = nullptr;
	waitSemaphores[3].semaphore = vk_state->imageUploadSemaphore.handle;
	waitSemaphores[3].value = vk_state->imageUploadSemaphore.submitValue;
	waitSemaphores[3].stageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
	waitSemaphores[3].deviceIndex = 0;

	#define SIGNAL_SEMAPHORE_COUNT 2
	VkSemaphoreSubmitInfo signalSemaphores[SIGNAL_SEMAPHORE_COUNT] = {};
	signalSemaphores[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	signalSemaphores[0].pNext = nullptr;
	signalSemaphores[0].semaphore = vk_state->renderFinishedSemaphoresDarray[vk_state->currentFrame];
	signalSemaphores[0].value = 0;
	signalSemaphores[0].stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	signalSemaphores[0].deviceIndex = 0;

	vk_state->frameSemaphore.submitValue++;
	signalSemaphores[1].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	signalSemaphores[1].pNext = nullptr;
	signalSemaphores[1].semaphore = vk_state->frameSemaphore.handle;
	signalSemaphores[1].value = vk_state->frameSemaphore.submitValue;
	signalSemaphores[1].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	signalSemaphores[1].deviceIndex = 0;

	SubmitCommandBuffers(WAIT_SEMAPHORE_COUNT, waitSemaphores, SIGNAL_SEMAPHORE_COUNT, signalSemaphores, 1, vk_state->commandBuffers[vk_state->currentFrame], nullptr);

	VkSwapchainKHR swapchains[] = { vk_state->swapchain };

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &vk_state->renderFinishedSemaphoresDarray[vk_state->currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &vk_state->currentSwapchainImageIndex;
	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(vk_state->presentQueue, &presentInfo);

	vk_state->currentFrame = (vk_state->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void UpdateGlobalUniforms(GlobalUniformObject* globalUniformObject, Texture texture)
{
	UpdateDescriptorSets(vk_state->currentFrame, (VulkanImage*)texture.internalState); /// HACK: make gfx pipelines configurable
	MemCopy(vk_state->uniformBuffersMappedDarray[vk_state->currentFrame], globalUniformObject, sizeof(GlobalUniformObject));
}

void DrawIndexed(VertexBuffer _vertexBuffer, IndexBuffer _indexBuffer, PushConstantObject* pPushConstantValues)
{
	VkCommandBuffer currentCommandBuffer = vk_state->commandBuffers[vk_state->currentFrame]->handle;

	VulkanVertexBuffer* vertexBuffer = (VulkanVertexBuffer*)_vertexBuffer.internalState;
	VulkanIndexBuffer* indexBuffer = (VulkanIndexBuffer*)_indexBuffer.internalState;

	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(currentCommandBuffer, 0, 1, &vertexBuffer->handle, offsets);

	vkCmdBindIndexBuffer(currentCommandBuffer, indexBuffer->handle, 0, VK_INDEX_TYPE_UINT32);

	vkCmdPushConstants(currentCommandBuffer, vk_state->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantObject), pPushConstantValues);

	vkCmdDrawIndexed(currentCommandBuffer, (u32)indexBuffer->indexCount, 1, 0, 0, 0);
}

static bool OnWindowResize(EventCode type, EventData data)
{
	vk_state->shouldRecreateSwapchain = true;
	return false;
}

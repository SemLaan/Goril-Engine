#include "../renderer.h"

#include "containers/darray.h"
#include "core/asserts.h"
#include "core/event.h"
#include "core/logger.h"
#include "core/meminc.h"

#include "vulkan_command_buffer.h"
#include "vulkan_debug_tools.h"
#include "vulkan_graphics_pipeline.h"
#include "vulkan_platform.h"
#include "vulkan_queues.h"
#include "vulkan_renderpass.h"
#include "vulkan_swapchain.h"
#include "vulkan_sync_objects.h"
#include "vulkan_types.h"
#include "vulkan_utils.h"

// TODO: remove these
static void Init2DRenderer();
static void Shutdown2DRenderer();

RendererState* vk_state = nullptr;

static bool OnWindowResize(EventCode type, EventData data);

bool InitializeRenderer()
{
    GRASSERT_DEBUG(vk_state == nullptr); // If this triggers init got called twice
    GRINFO("Initializing renderer subsystem...");

    vk_state = AlignedAlloc(GetGlobalAllocator(), sizeof(RendererState), 64 /*cache line*/, MEM_TAG_RENDERER_SUBSYS);
    ZeroMem(vk_state, sizeof(*vk_state));
    CreateFreelistAllocator("renderer allocator", GetGlobalAllocator(), KiB * 100, &vk_state->rendererAllocator);
    CreateBumpAllocator("renderer bump allocator", vk_state->rendererAllocator, KiB * 5, &vk_state->rendererBumpAllocator);
    CreatePoolAllocator("renderer resource destructor pool", vk_state->rendererAllocator, RENDER_POOL_BLOCK_SIZE_32, 30, &vk_state->poolAllocator32B);
    CreatePoolAllocator("Renderer resource acquisition pool", vk_state->rendererAllocator, QUEUE_ACQUISITION_POOL_BLOCK_SIZE, 30, &vk_state->resourceAcquisitionPool);

    vk_state->vkAllocator = nullptr; // TODO: add something that tracks vulkan API allocations in debug mode

    vk_state->currentInFlightFrameIndex = 0;
    vk_state->shouldRecreateSwapchain = false;

    RegisterEventListener(EVCODE_WINDOW_RESIZED, OnWindowResize);

// ================== Getting required instance extensions and layers ================================
#define MAX_INSTANCE_EXTENSIONS 10
#define MAX_INSTANCE_LAYERS 10
    // Getting required extensions
    const char* requiredInstanceExtensions[MAX_INSTANCE_EXTENSIONS];
    u32 requiredInstanceExtensionCount = 0;

    requiredInstanceExtensions[requiredInstanceExtensionCount] = VK_KHR_SURFACE_EXTENSION_NAME;
    requiredInstanceExtensionCount++;
    GetPlatformExtensions(&requiredInstanceExtensionCount, requiredInstanceExtensions);
    ADD_DEBUG_INSTANCE_EXTENSIONS(requiredInstanceExtensions, requiredInstanceExtensionCount);

    // Getting required layers
    const char* requiredInstanceLayers[MAX_INSTANCE_LAYERS];
    u32 requiredInstanceLayerCount = 0;
    ADD_DEBUG_INSTANCE_LAYERS(requiredInstanceLayers, requiredInstanceLayerCount);

    // ============================================================================================================================================================
    // =========================================================== Creating instance ==============================================================================
    // ============================================================================================================================================================
    {
        // ================ App info =============================================
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.pApplicationName = "Test app";
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.pEngineName = "Goril";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        {
            // Checking if required extensions are available
            u32 availableExtensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
            VkExtensionProperties* availableExtensionsDarray = (VkExtensionProperties*)DarrayCreateWithSize(sizeof(VkExtensionProperties), availableExtensionCount, vk_state->rendererAllocator, MEM_TAG_RENDERER_SUBSYS);
            vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensionsDarray);

            bool extensionsAvailable = CheckRequiredExtensions(requiredInstanceExtensionCount, requiredInstanceExtensions, availableExtensionCount, availableExtensionsDarray);

            DarrayDestroy(availableExtensionsDarray);

            if (!extensionsAvailable)
            {
                GRFATAL("Couldn't find required Vulkan extensions");
                return false;
            }
            else
                GRTRACE("Required Vulkan extensions found");
        }

        {
            // Checking if required layers are available
            u32 availableLayerCount = 0;
            vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
            VkLayerProperties* availableLayersDarray = (VkLayerProperties*)DarrayCreate(sizeof(VkLayerProperties), availableLayerCount, vk_state->rendererAllocator, MEM_TAG_RENDERER_SUBSYS);
            vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayersDarray);

            bool layersAvailable = CheckRequiredLayers(requiredInstanceLayerCount, requiredInstanceLayers, availableLayerCount, availableLayersDarray);

            DarrayDestroy(availableLayersDarray);

            if (!layersAvailable)
            {
                GRFATAL("Couldn't find required Vulkan layers");
                return false;
            }
            else
                GRTRACE("Required Vulkan layers found");
        }

        // ================== Creating instance =================================
        {
            VkInstanceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

            GetDebugMessengerCreateInfo(createInfo.pNext);
            createInfo.flags = 0;
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledLayerCount = requiredInstanceLayerCount;
            createInfo.ppEnabledLayerNames = requiredInstanceLayers;
            createInfo.enabledExtensionCount = requiredInstanceExtensionCount;
            createInfo.ppEnabledExtensionNames = requiredInstanceExtensions;

            VkResult result = vkCreateInstance(&createInfo, vk_state->vkAllocator, &vk_state->instance);

            if (result != VK_SUCCESS)
            {
                GRFATAL("Failed to create Vulkan instance");
                return false;
            }
        }

        GRTRACE("Vulkan instance created");
    }

    // ============================================================================================================================================================
    // =============== Creating debug messenger ===================================================================================================================
    // ============================================================================================================================================================
    CreateDebugMessenger();

    // ============================================================================================================================================================
    // ================ Creating a surface ========================================================================================================================
    // ============================================================================================================================================================
    if (!PlatformCreateSurface(vk_state->instance, vk_state->vkAllocator, &vk_state->surface))
    {
        GRFATAL("Failed to create Vulkan surface");
        return false;
    }

// ============================================================================================================================================================
// ================ Getting a physical device =================================================================================================================
// ============================================================================================================================================================
#define MAX_DEVICE_EXTENSIONS 10
    const char* requiredDeviceExtensions[MAX_DEVICE_EXTENSIONS];
    u32 requiredDeviceExtensionCount = 0;

    requiredDeviceExtensions[requiredDeviceExtensionCount + 0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    requiredDeviceExtensions[requiredDeviceExtensionCount + 1] = VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME;
    requiredDeviceExtensions[requiredDeviceExtensionCount + 2] = VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME;
    requiredDeviceExtensionCount += 3;

    {
        vk_state->physicalDevice = VK_NULL_HANDLE;

        u32 deviceCount = 0;
        vkEnumeratePhysicalDevices(vk_state->instance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            GRFATAL("No Vulkan devices found");
            return false;
        }

        VkPhysicalDevice* availableDevicesDarray = (VkPhysicalDevice*)DarrayCreateWithSize(sizeof(VkPhysicalDevice), deviceCount, vk_state->rendererAllocator, MEM_TAG_RENDERER_SUBSYS); // TODO: different allocator
        vkEnumeratePhysicalDevices(vk_state->instance, &deviceCount, availableDevicesDarray);

        /// TODO: better device selection
        for (u32 i = 0; i < DarrayGetSize(availableDevicesDarray); ++i)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(availableDevicesDarray[i], &properties);
            bool isDiscrete = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            bool extensionsAvailable;

            { // Checking if the device has the required extensions
                u32 availableExtensionCount = 0;
                vkEnumerateDeviceExtensionProperties(availableDevicesDarray[i], nullptr, &availableExtensionCount, nullptr);
                VkExtensionProperties* availableExtensionsDarray = (VkExtensionProperties*)DarrayCreateWithSize(sizeof(VkExtensionProperties), availableExtensionCount, g_Allocators->temporary, MEM_TAG_RENDERER_SUBSYS); // TODO: change from darray to just array
                vkEnumerateDeviceExtensionProperties(availableDevicesDarray[i], nullptr, &availableExtensionCount, availableExtensionsDarray);

                extensionsAvailable = CheckRequiredExtensions(requiredDeviceExtensionCount, requiredDeviceExtensions, availableExtensionCount, availableExtensionsDarray);

                DarrayDestroy(availableExtensionsDarray);
            }

            if (isDiscrete && extensionsAvailable)
            {
                GRTRACE("Device with required extensions, features and properties found");
                SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(availableDevicesDarray[i], vk_state->surface);
                if (DarrayGetSize(swapchainSupport.formatsDarray) != 0 && DarrayGetSize(swapchainSupport.presentModesDarray) != 0)
                {
                    vk_state->physicalDevice = availableDevicesDarray[i];
                    vk_state->swapchainSupport = swapchainSupport;
                    break;
                }
                DarrayDestroy(swapchainSupport.formatsDarray);
                DarrayDestroy(swapchainSupport.presentModesDarray);
            }
        }

        DarrayDestroy(availableDevicesDarray);

        if (vk_state->physicalDevice == VK_NULL_HANDLE)
        {
            GRFATAL("No suitable devices found");
            return false;
        }

        GRTRACE("Succesfully selected physical vulkan device");
    }

    // ============================================================================================================================================================
    // ================== Getting device queue families ===========================================================================================================
    // ============================================================================================================================================================
    {
        u32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vk_state->physicalDevice, &queueFamilyCount, nullptr);
        VkQueueFamilyProperties* availableQueueFamiliesDarray = (VkQueueFamilyProperties*)DarrayCreateWithSize(sizeof(VkQueueFamilyProperties), queueFamilyCount, vk_state->rendererAllocator, MEM_TAG_RENDERER_SUBSYS);
        vkGetPhysicalDeviceQueueFamilyProperties(vk_state->physicalDevice, &queueFamilyCount, availableQueueFamiliesDarray);

        vk_state->transferQueue.index = UINT32_MAX;

        for (u32 i = 0; i < queueFamilyCount; ++i)
        {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(vk_state->physicalDevice, i, vk_state->surface, &presentSupport);
            bool graphicsSupport = availableQueueFamiliesDarray[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
            bool transferSupport = availableQueueFamiliesDarray[i].queueFlags & VK_QUEUE_TRANSFER_BIT;
            if (graphicsSupport)
                vk_state->graphicsQueue.index = i;
            if (presentSupport)
                vk_state->presentQueueFamilyIndex = i;
            if (transferSupport && !graphicsSupport)
                vk_state->transferQueue.index = i;
        }

        if (vk_state->transferQueue.index == UINT32_MAX)
            vk_state->transferQueue.index = vk_state->graphicsQueue.index;
        /// TODO: check if the device even has queue families for all these things, if not fail startup (is this even required? i think implementations need at least transfer and graphics(?), and compute and present are implied by the existence of the extensions)
        DarrayDestroy(availableQueueFamiliesDarray);
    }

    // ============================================================================================================================================================
    // ===================== Creating logical device ==============================================================================================================
    // ============================================================================================================================================================
    {
        // ===================== Specifying queues for logical device =================================
        VkDeviceQueueCreateInfo* queueCreateInfosDarray = (VkDeviceQueueCreateInfo*)DarrayCreate(sizeof(VkDeviceQueueCreateInfo), 1, vk_state->rendererAllocator, MEM_TAG_RENDERER_SUBSYS); // TODO: switch allocator

        u32* uniqueQueueFamiliesDarray = (u32*)DarrayCreate(sizeof(u32), 5, vk_state->rendererAllocator, MEM_TAG_RENDERER_SUBSYS); // TODO: switch allocator
        if (!DarrayContains(uniqueQueueFamiliesDarray, &vk_state->graphicsQueue.index))
            uniqueQueueFamiliesDarray = (u32*)DarrayPushback(uniqueQueueFamiliesDarray, &vk_state->graphicsQueue.index);
        if (!DarrayContains(uniqueQueueFamiliesDarray, &vk_state->presentQueueFamilyIndex))
            uniqueQueueFamiliesDarray = (u32*)DarrayPushback(uniqueQueueFamiliesDarray, &vk_state->presentQueueFamilyIndex);
        if (!DarrayContains(uniqueQueueFamiliesDarray, &vk_state->transferQueue.index))
            uniqueQueueFamiliesDarray = (u32*)DarrayPushback(uniqueQueueFamiliesDarray, &vk_state->transferQueue.index);

        f32 queuePriority = 1.0f;

        for (u32 i = 0; i < DarrayGetSize(uniqueQueueFamiliesDarray); ++i)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.pNext = nullptr;
            queueCreateInfo.flags = 0;
            queueCreateInfo.queueFamilyIndex = uniqueQueueFamiliesDarray[i];
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfosDarray = (VkDeviceQueueCreateInfo*)DarrayPushback(queueCreateInfosDarray, &queueCreateInfo);
        }

        DarrayDestroy(uniqueQueueFamiliesDarray);

        // ===================== Specifying features for logical device ==============================
        VkPhysicalDeviceFeatures deviceFeatures = {};
        /// TODO: add required device features here, these should be retrieved from the application config

        /// Put new extension features above here and make the extension feature under this point to that new feature
        VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeatures = {};
        timelineSemaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
        timelineSemaphoreFeatures.pNext = nullptr;
        timelineSemaphoreFeatures.timelineSemaphore = VK_TRUE;

        VkPhysicalDeviceSynchronization2Features synchronization2Features = {};
        synchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        synchronization2Features.pNext = &timelineSemaphoreFeatures;
        synchronization2Features.synchronization2 = VK_TRUE;

        VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &synchronization2Features;
        deviceFeatures2.features = deviceFeatures;

        // ===================== Creating logical device =============================================
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &deviceFeatures2;
        createInfo.flags = 0;
        createInfo.queueCreateInfoCount = DarrayGetSize(queueCreateInfosDarray);
        createInfo.pQueueCreateInfos = queueCreateInfosDarray;
        createInfo.enabledLayerCount = requiredInstanceLayerCount;
        createInfo.ppEnabledLayerNames = requiredInstanceLayers;
        createInfo.enabledExtensionCount = requiredDeviceExtensionCount;
        createInfo.ppEnabledExtensionNames = requiredDeviceExtensions;
        createInfo.pEnabledFeatures = nullptr;

        u32 result = vkCreateDevice(vk_state->physicalDevice, &createInfo, vk_state->vkAllocator, &vk_state->device);

        DarrayDestroy(queueCreateInfosDarray);

        if (result != VK_SUCCESS)
        {
            GRFATAL("Failed to create Vulkan logical device");
            return false;
        }

        GRTRACE("Succesfully created vulkan logical device");
    }

    // ============================================================================================================================================================
    // ===================== sets up queues and command pools =====================================================================================================
    // ============================================================================================================================================================
    if (!CreateQueues())
        return false;

    // ============================================================================================================================================================
    // ============================ Allocate graphics command buffers =============================================================================================
    // ============================================================================================================================================================
    for (i32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (!AllocateCommandBuffer(&vk_state->graphicsQueue, &vk_state->graphicsCommandBuffers[i]))
            return false;
    }

    // ============================================================================================================================================================
    // ================================ Create sync objects =======================================================================================================
    // ============================================================================================================================================================
    if (!CreateSyncObjects())
        return false;

    // ============================================================================================================================================================
    // ======================== Creating the swapchain ============================================================================================================
    // ============================================================================================================================================================
    if (!CreateSwapchain(vk_state))
        return false;

    // TODO: remove the swapchain framebuffers and renderpass by making it dynamic, also remove this other stuff and add it to the startup of the 2d renderer
    // ========================== Creating renderpass ==============================================
    if (!CreateRenderpass(vk_state))
        return false;

    // ======================== Creating graphics pipeline ============================================
    if (!CreateGraphicsPipeline())
        return false;

    // ======================= Create swapchain framebuffers ============================================
    if (!CreateSwapchainFramebuffers(vk_state))
        return false;

    vk_state->requestedQueueAcquisitionOperationsDarray = (VkDependencyInfo**)DarrayCreate(sizeof(VkDependencyInfo*), 10, vk_state->rendererAllocator, MEM_TAG_RENDERER_SUBSYS); /// TODO: change allocator to renderer local allocator (when it exists)

    Init2DRenderer();

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

    Shutdown2DRenderer();

    UnregisterEventListener(EVCODE_WINDOW_RESIZED, OnWindowResize);

    if (vk_state->device)
        vkDeviceWaitIdle(vk_state->device);

    if (vk_state->graphicsQueue.resourcesPendingDestructionDarray)
        TryDestroyResourcesPendingDestruction();

    if (vk_state->requestedQueueAcquisitionOperationsDarray)
        DarrayDestroy(vk_state->requestedQueueAcquisitionOperationsDarray);

    // ====================== Destroying swapchain framebuffers if they were created ================================
    DestroySwapchainFramebuffers(vk_state);

    // ====================== Destroying graphics pipeline if it was created ================================
    DestroyGraphicsPipeline();

    // ======================== Destroying renderpass if it was created =====================================
    DestroyRenderpass(vk_state);

    // ============================================================================================================================================================
    // ====================== Destroying swapchain if it was created ==============================================================================================
    // ============================================================================================================================================================
    DestroySwapchain(vk_state);

    // ============================================================================================================================================================
    // ================================ Destroy sync objects if they were created =================================================================================
    // ============================================================================================================================================================
    DestroySyncObjects();

    // ============================================================================================================================================================
    // =================================== Free command buffers ===================================================================================================
    // ============================================================================================================================================================
    if (vk_state->graphicsCommandBuffers)
    {
        for (i32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            FreeCommandBuffer(vk_state->graphicsCommandBuffers[i]);
        }
    }

    // ============================================================================================================================================================
    // ===================== destroys queues and command pools ====================================================================================================
    // ============================================================================================================================================================
    DestroyQueues();

    // ============================================================================================================================================================
    // ===================== Destroying logical device if it was created ==========================================================================================
    // ============================================================================================================================================================
    if (vk_state->device)
        vkDestroyDevice(vk_state->device, vk_state->vkAllocator);
    if (vk_state->swapchainSupport.formatsDarray)
        DarrayDestroy(vk_state->swapchainSupport.formatsDarray);
    if (vk_state->swapchainSupport.presentModesDarray)
        DarrayDestroy(vk_state->swapchainSupport.presentModesDarray);

    // ============================================================================================================================================================
    // ======================= Destroying the surface if it was created ===========================================================================================
    // ============================================================================================================================================================
    if (vk_state->surface)
        vkDestroySurfaceKHR(vk_state->instance, vk_state->surface, vk_state->vkAllocator);

    // ============================================================================================================================================================
    // ===================== Destroying debug messenger if it was created =========================================================================================
    // ============================================================================================================================================================
    DestroyDebugMessenger();

    // ============================================================================================================================================================
    // ======================= Destroying instance if it was created ==============================================================================================
    // ============================================================================================================================================================
    if (vk_state->instance)
        vkDestroyInstance(vk_state->instance, vk_state->vkAllocator);

    DestroyPoolAllocator(vk_state->resourceAcquisitionPool);
    DestroyPoolAllocator(vk_state->poolAllocator32B);
    DestroyBumpAllocator(vk_state->rendererBumpAllocator);
    DestroyFreelistAllocator(vk_state->rendererAllocator);
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

// TODO: change where the actual scene rendering happens
static void Preprocess2DSceneData();
static void Render2DScene();

bool RenderFrame()
{
    // Destroy temporary resources that the GPU has finished with (e.g. staging buffers, etc.)
    TryDestroyResourcesPendingDestruction();

    // Recreating the swapchain if the window has been resized
    if (vk_state->shouldRecreateSwapchain)
        RecreateSwapchain();

    // ================================= Waiting for rendering resources to become available ==============================================================
    // The GPU can work on multiple frames simultaneously (i.e. multiple frames can be "in flight"), but each frame has it's own resources
    // that the GPU needs while it's rendering a frame. So we need to wait for one of those sets of resources to become available again (command buffers and binary semaphores).
    VkSemaphoreWaitInfo semaphoreWaitInfo = {};
    semaphoreWaitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    semaphoreWaitInfo.pNext = nullptr;
    semaphoreWaitInfo.flags = 0;
    semaphoreWaitInfo.semaphoreCount = 1;
    semaphoreWaitInfo.pSemaphores = &vk_state->frameSemaphore.handle;
    u64 waitForValue = vk_state->frameSemaphore.submitValue - (MAX_FRAMES_IN_FLIGHT - 1);
    semaphoreWaitInfo.pValues = &waitForValue;

    vkWaitSemaphores(vk_state->device, &semaphoreWaitInfo, UINT64_MAX);

    // Getting the next image from the swapchain (doesn't block the CPU and only blocks the GPU if there's no image available (which only happens in certain present modes with certain buffer counts))
    VkResult result = vkAcquireNextImageKHR(vk_state->device, vk_state->swapchain, UINT64_MAX, vk_state->imageAvailableSemaphoresDarray[vk_state->currentInFlightFrameIndex], VK_NULL_HANDLE, &vk_state->currentSwapchainImageIndex);

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

    // =================================== Preprocess user scene data before recording actual commands ===========================================
    Preprocess2DSceneData();

    // ===================================== Begin command buffer recording =========================================
    ResetAndBeginCommandBuffer(vk_state->graphicsCommandBuffers[vk_state->currentInFlightFrameIndex]);
    VkCommandBuffer currentCommandBuffer = vk_state->graphicsCommandBuffers[vk_state->currentInFlightFrameIndex].handle;

    // =============================== acquire ownership of all uploaded resources =======================================
    for (u32 i = 0; i < DarrayGetSize(vk_state->requestedQueueAcquisitionOperationsDarray); ++i)
    {
        vkCmdPipelineBarrier2(currentCommandBuffer, vk_state->requestedQueueAcquisitionOperationsDarray[i]);
        Free(vk_state->resourceAcquisitionPool, vk_state->requestedQueueAcquisitionOperationsDarray[i]);
    }

    DarraySetSize(vk_state->requestedQueueAcquisitionOperationsDarray, 0);

    // ==================================== Begin renderpass ==============================================
    /// TODO: begin renderpass function and remove renderpass objects
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

    // TODO: maybe make this static? (this would mean that all pipelines would have to be recreated upon window resize, but.. who cares? better than at runtime especially since otherwise they have to be set every renderpass)
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

    // =========================================== Render user submitted scene ================================
    // TODO: binding descriptor sets and binding the pipeline should probably be a part of the Render2DScene function
    // Binding global descriptor set
    vkCmdBindDescriptorSets(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_state->pipelineLayout, 0, 1, &vk_state->uniformDescriptorSetsDarray[vk_state->currentInFlightFrameIndex], 0, nullptr);

    /// TODO: bind graphics pipeline function (and abstracting graphics pipelines in general)
    vkCmdBindPipeline(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_state->graphicsPipeline);

    Render2DScene();

    // ==================================== End renderpass =================================================
    /// TODO: end renderpass function
    vkCmdEndRenderPass(vk_state->graphicsCommandBuffers[vk_state->currentInFlightFrameIndex].handle);

    // ================================= End command buffer recording ==================================================
    EndCommandBuffer(vk_state->graphicsCommandBuffers[vk_state->currentInFlightFrameIndex]);

    // =================================== Submitting command buffer ==============================================
    // With all the synchronization that that entails...
#define WAIT_SEMAPHORE_COUNT 4 // 1 swapchain image acquisition, 3 resourse upload waits
    VkSemaphoreSubmitInfo waitSemaphores[WAIT_SEMAPHORE_COUNT] = {};

    // Swapchain image acquisition semaphore
    waitSemaphores[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    waitSemaphores[0].pNext = nullptr;
    waitSemaphores[0].semaphore = vk_state->imageAvailableSemaphoresDarray[vk_state->currentInFlightFrameIndex];
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
    signalSemaphores[0].semaphore = vk_state->renderFinishedSemaphoresDarray[vk_state->currentInFlightFrameIndex];
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

    // Submitting the command buffer which allows the GPU to actually start working on this frame
    SubmitCommandBuffers(WAIT_SEMAPHORE_COUNT, waitSemaphores, SIGNAL_SEMAPHORE_COUNT, signalSemaphores, 1, &vk_state->graphicsCommandBuffers[vk_state->currentInFlightFrameIndex], nullptr);

    // ============================== Telling the GPU to present this frame (after it's rendered of course, synced with a binary semaphore) =================================
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &vk_state->renderFinishedSemaphoresDarray[vk_state->currentInFlightFrameIndex];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vk_state->swapchain;
    presentInfo.pImageIndices = &vk_state->currentSwapchainImageIndex;
    presentInfo.pResults = nullptr;

    // When using mailbox present mode, vulkan will take care of skipping the presentation of this frame if another one is already finished
    vkQueuePresentKHR(vk_state->presentQueue, &presentInfo);

    vk_state->currentInFlightFrameIndex = (vk_state->currentInFlightFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

    return true;
}

typedef struct Renderer2DState
{
    SceneRenderData2D currentRenderData;
    GlobalUniformObject currentGlobalUBO;
    VertexBuffer quadVertexBuffer;
    VertexBuffer instancedBuffer;
    IndexBuffer quadIndexBuffer;
} Renderer2DState;

static Renderer2DState* renderer2DState = nullptr;

static void Init2DRenderer()
{
    renderer2DState = Alloc(vk_state->rendererBumpAllocator, sizeof(*renderer2DState), MEM_TAG_RENDERER_SUBSYS);

#define QUAD_VERT_COUNT 4
    Vertex quadVertices[QUAD_VERT_COUNT] =
        {
            {{0, 0, 0}, {0, 0, 0}, {0, 0}},
            {{0, 1, 0}, {0, 0, 0}, {0, 1}},
            {{1, 0, 0}, {0, 0, 0}, {1, 0}},
            {{1, 1, 0}, {0, 0, 0}, {1, 1}},
        };

    renderer2DState->quadVertexBuffer = VertexBufferCreate(quadVertices, sizeof(quadVertices));

#define QUAD_INDEX_COUNT 6
    u32 quadIndices[QUAD_INDEX_COUNT] =
        {
            0, 1, 2,
            2, 1, 3};

    renderer2DState->quadIndexBuffer = IndexBufferCreate(quadIndices, QUAD_INDEX_COUNT);

    renderer2DState->instancedBuffer = VertexBufferCreate(nullptr, 100 * sizeof(SpriteInstance));
}

static void Shutdown2DRenderer()
{
    IndexBufferDestroy(renderer2DState->quadIndexBuffer);
    VertexBufferDestroy(renderer2DState->instancedBuffer);
    VertexBufferDestroy(renderer2DState->quadVertexBuffer);

    Free(vk_state->rendererBumpAllocator, renderer2DState);
}

static void Preprocess2DSceneData()
{
    // ========================== Preprocessing camera ================================
    renderer2DState->currentGlobalUBO.projView = renderer2DState->currentRenderData.camera;

    // =========================== preprocessing quads =================================
    u32 spriteCount = DarrayGetSize(renderer2DState->currentRenderData.spriteRenderInfoDarray);
    GRASSERT(spriteCount > 0);

    // TODO: bind textures
    // TODO: set uniforms (textures and camera)

    SpriteInstance* instanceData = Alloc(vk_state->rendererAllocator, sizeof(*instanceData) * spriteCount, MEM_TAG_RENDERER_SUBSYS);

    for (u32 i = 0; i < spriteCount; ++i)
    {
        instanceData[i].model = renderer2DState->currentRenderData.spriteRenderInfoDarray[i].model;
        instanceData[i].textureIndex = 0; // TODO: fill instanced buffer with texture indices
    }

    VertexBufferUpdate(renderer2DState->instancedBuffer, instanceData, spriteCount * sizeof(*instanceData));

    Free(vk_state->rendererAllocator, instanceData);

    // =============================== Updating descriptor sets ==================================
    MemCopy(vk_state->uniformBuffersMappedDarray[vk_state->currentInFlightFrameIndex], &renderer2DState->currentGlobalUBO, sizeof(GlobalUniformObject));
    UpdateDescriptorSets(vk_state->currentInFlightFrameIndex, (VulkanImage*)renderer2DState->currentRenderData.spriteRenderInfoDarray[0].texture.internalState); // TODO: make gfx pipelines configurable
}

static void Render2DScene()
{
    u32 spriteCount = DarrayGetSize(renderer2DState->currentRenderData.spriteRenderInfoDarray);

    VkCommandBuffer currentCommandBuffer = vk_state->graphicsCommandBuffers[vk_state->currentInFlightFrameIndex].handle;

    VulkanVertexBuffer* quadBuffer = renderer2DState->quadVertexBuffer.internalState;
    VulkanIndexBuffer* indexBuffer = renderer2DState->quadIndexBuffer.internalState;
    VulkanVertexBuffer* instancedBuffer = renderer2DState->instancedBuffer.internalState;

    VkBuffer vertexBuffers[2] = {quadBuffer->handle, instancedBuffer->handle};

    VkDeviceSize offsets[2] = {0, 0};
    vkCmdBindVertexBuffers(currentCommandBuffer, 0, 2, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(currentCommandBuffer, indexBuffer->handle, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(currentCommandBuffer, indexBuffer->indexCount, spriteCount, 0, 0, 0);

    DarrayDestroy(renderer2DState->currentRenderData.spriteRenderInfoDarray);
}

void Submit2DScene(SceneRenderData2D sceneData)
{
    renderer2DState->currentRenderData = sceneData;
}

void DrawIndexed(VertexBuffer _vertexBuffer, IndexBuffer _indexBuffer, PushConstantObject* pPushConstantValues)
{
    VkCommandBuffer currentCommandBuffer = vk_state->graphicsCommandBuffers[vk_state->currentInFlightFrameIndex].handle;

    VulkanVertexBuffer* vertexBuffer = (VulkanVertexBuffer*)_vertexBuffer.internalState;
    VulkanIndexBuffer* indexBuffer = (VulkanIndexBuffer*)_indexBuffer.internalState;

    VkDeviceSize offsets[] = {0};
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

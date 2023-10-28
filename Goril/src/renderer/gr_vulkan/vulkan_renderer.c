#include "../renderer.h"

#include "containers/darray.h"
#include "core/asserts.h"
#include "core/event.h"
#include "core/logger.h"
#include "core/meminc.h"

#include "renderer/texture.h"
#include "vulkan_command_buffer.h"
#include "vulkan_debug_tools.h"
#include "vulkan_graphics_pipeline.h"
#include "vulkan_platform.h"
#include "vulkan_swapchain.h"
#include "vulkan_types.h"
#include "vulkan_utils.h"
#include "vulkan_2D_renderer.h"


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
    requiredDeviceExtensions[requiredDeviceExtensionCount + 3] = VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME;
    requiredDeviceExtensionCount += 4;

    {
        vk_state->physicalDevice = VK_NULL_HANDLE;

        u32 deviceCount = 0;
        vkEnumeratePhysicalDevices(vk_state->instance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            GRFATAL("No Vulkan devices found");
            return false;
        }

        VkPhysicalDevice* availableDevices = Alloc(vk_state->rendererAllocator, sizeof(*availableDevices) * deviceCount, MEM_TAG_RENDERER_SUBSYS);
        vkEnumeratePhysicalDevices(vk_state->instance, &deviceCount, availableDevices);

        /// TODO: better device selection
        for (u32 i = 0; i < deviceCount; ++i)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(availableDevices[i], &properties);
            bool isDiscrete = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            bool extensionsAvailable;

            { // Checking if the device has the required extensions
                u32 availableExtensionCount = 0;
                vkEnumerateDeviceExtensionProperties(availableDevices[i], nullptr, &availableExtensionCount, nullptr);
                VkExtensionProperties* availableExtensionsDarray = (VkExtensionProperties*)DarrayCreateWithSize(sizeof(VkExtensionProperties), availableExtensionCount, g_Allocators->temporary, MEM_TAG_RENDERER_SUBSYS); // TODO: change from darray to just array
                vkEnumerateDeviceExtensionProperties(availableDevices[i], nullptr, &availableExtensionCount, availableExtensionsDarray);

                extensionsAvailable = CheckRequiredExtensions(requiredDeviceExtensionCount, requiredDeviceExtensions, availableExtensionCount, availableExtensionsDarray);

                DarrayDestroy(availableExtensionsDarray);
            }

            if (isDiscrete && extensionsAvailable)
            {
                GRTRACE("Device with required extensions, features and properties found");
                SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(availableDevices[i], vk_state->surface);
                if (swapchainSupport.formatCount != 0 && swapchainSupport.presentModeCount != 0)
                {
                    vk_state->physicalDevice = availableDevices[i];
                    vk_state->swapchainSupport = swapchainSupport;
                    break;
                }
                Free(vk_state->rendererAllocator, swapchainSupport.formats);
                Free(vk_state->rendererAllocator, swapchainSupport.presentModes);
            }
        }

        Free(vk_state->rendererAllocator, availableDevices);

        if (vk_state->physicalDevice == VK_NULL_HANDLE)
        {
            GRFATAL("No suitable devices found");
            return false;
        }

        GRTRACE("Successfully selected physical vulkan device");
    }

    // ============================================================================================================================================================
    // ================== Getting device queue families ===========================================================================================================
    // ============================================================================================================================================================
    {
        u32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vk_state->physicalDevice, &queueFamilyCount, nullptr);
        VkQueueFamilyProperties* availableQueueFamilies = Alloc(vk_state->rendererAllocator, sizeof(*availableQueueFamilies) * queueFamilyCount, MEM_TAG_RENDERER_SUBSYS);
        vkGetPhysicalDeviceQueueFamilyProperties(vk_state->physicalDevice, &queueFamilyCount, availableQueueFamilies);

        vk_state->transferQueue.index = UINT32_MAX;

        for (u32 i = 0; i < queueFamilyCount; ++i)
        {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(vk_state->physicalDevice, i, vk_state->surface, &presentSupport);
            bool graphicsSupport = availableQueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
            bool transferSupport = availableQueueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT;
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
        Free(vk_state->rendererAllocator, availableQueueFamilies);
    }

    // ============================================================================================================================================================
    // ===================== Creating logical device ==============================================================================================================
    // ============================================================================================================================================================
    {
        // ===================== Specifying queues for logical device =================================
        u32* uniqueQueueFamiliesDarray = DarrayCreate(sizeof(u32), 5, vk_state->rendererAllocator, MEM_TAG_RENDERER_SUBSYS);
        if (!DarrayContains(uniqueQueueFamiliesDarray, &vk_state->graphicsQueue.index))
            uniqueQueueFamiliesDarray = DarrayPushback(uniqueQueueFamiliesDarray, &vk_state->graphicsQueue.index);
        if (!DarrayContains(uniqueQueueFamiliesDarray, &vk_state->presentQueueFamilyIndex))
            uniqueQueueFamiliesDarray = DarrayPushback(uniqueQueueFamiliesDarray, &vk_state->presentQueueFamilyIndex);
        if (!DarrayContains(uniqueQueueFamiliesDarray, &vk_state->transferQueue.index))
            uniqueQueueFamiliesDarray = DarrayPushback(uniqueQueueFamiliesDarray, &vk_state->transferQueue.index);

        const f32 queuePriority = 1.0f;

        u32 uniqueQueueCount = DarrayGetSize(uniqueQueueFamiliesDarray);

        VkDeviceQueueCreateInfo* queueCreateInfos = Alloc(vk_state->rendererAllocator, sizeof(*queueCreateInfos) * uniqueQueueCount, MEM_TAG_RENDERER_SUBSYS);

        for (u32 i = 0; i < uniqueQueueCount; ++i)
        {
            queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfos[i].pNext = nullptr;
            queueCreateInfos[i].flags = 0;
            queueCreateInfos[i].queueFamilyIndex = uniqueQueueFamiliesDarray[i];
            queueCreateInfos[i].queueCount = 1;
            queueCreateInfos[i].pQueuePriorities = &queuePriority;
        }

        DarrayDestroy(uniqueQueueFamiliesDarray);

        // ===================== Specifying features for logical device ==============================
        VkPhysicalDeviceFeatures deviceFeatures = {};
        /// TODO: add required device features here, these should be retrieved from the application config

        /// Put new extension features above here and make the extension feature under this point to that new feature
        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {};
        dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        dynamicRenderingFeatures.pNext = nullptr;
        dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

        VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeatures = {};
        timelineSemaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
        timelineSemaphoreFeatures.pNext = &dynamicRenderingFeatures;
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
        createInfo.queueCreateInfoCount = uniqueQueueCount;
        createInfo.pQueueCreateInfos = queueCreateInfos;
        createInfo.enabledLayerCount = requiredInstanceLayerCount;
        createInfo.ppEnabledLayerNames = requiredInstanceLayers;
        createInfo.enabledExtensionCount = requiredDeviceExtensionCount;
        createInfo.ppEnabledExtensionNames = requiredDeviceExtensions;
        createInfo.pEnabledFeatures = nullptr;

        u32 result = vkCreateDevice(vk_state->physicalDevice, &createInfo, vk_state->vkAllocator, &vk_state->device);

        Free(vk_state->rendererAllocator, queueCreateInfos);

        if (result != VK_SUCCESS)
        {
            GRFATAL("Failed to create Vulkan logical device");
            return false;
        }

        GRTRACE("Successfully created vulkan logical device");
    }

    // ============================================================================================================================================================
    // ===================== sets up queues and command pools =====================================================================================================
    // ============================================================================================================================================================
    {
        // =================== Getting the device queues ======================================================
        // Present family queue
        vkGetDeviceQueue(vk_state->device, vk_state->presentQueueFamilyIndex, 0, &vk_state->presentQueue);

        /// TODO: get compute queue
        // Graphics, transfer and (in the future) compute queue
        vkGetDeviceQueue(vk_state->device, vk_state->graphicsQueue.index, 0, &vk_state->graphicsQueue.handle);
        vk_state->graphicsQueue.resourcesPendingDestructionDarray = DarrayCreate(sizeof(ResourceDestructionInfo), 20, vk_state->rendererAllocator, MEM_TAG_RENDERER_SUBSYS);

        vkGetDeviceQueue(vk_state->device, vk_state->transferQueue.index, 0, &vk_state->transferQueue.handle);
        vk_state->transferQueue.resourcesPendingDestructionDarray = DarrayCreate(sizeof(ResourceDestructionInfo), 20, vk_state->rendererAllocator, MEM_TAG_RENDERER_SUBSYS);

        // ==================== Creating command pools for each of the queue families =============================
        VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.pNext = nullptr;
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = vk_state->graphicsQueue.index;

        if (VK_SUCCESS != vkCreateCommandPool(vk_state->device, &commandPoolCreateInfo, vk_state->vkAllocator, &vk_state->graphicsQueue.commandPool))
        {
            GRFATAL("Failed to create Vulkan graphics command pool");
            return false;
        }

        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        commandPoolCreateInfo.queueFamilyIndex = vk_state->transferQueue.index;

        if (VK_SUCCESS != vkCreateCommandPool(vk_state->device, &commandPoolCreateInfo, vk_state->vkAllocator, &vk_state->transferQueue.commandPool))
        {
            GRFATAL("Failed to create Vulkan transfer command pool");
            return false;
        }

        /// TODO: create compute command pool

        // Create semaphores
        vk_state->graphicsQueue.semaphore.submitValue = 0;
        vk_state->transferQueue.semaphore.submitValue = 0;

        VkSemaphoreTypeCreateInfo semaphoreTypeInfo = {};
        semaphoreTypeInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        semaphoreTypeInfo.pNext = nullptr;
        semaphoreTypeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        semaphoreTypeInfo.initialValue = 0;

        VkSemaphoreCreateInfo timelineSemaphoreCreateInfo = {};
        timelineSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        timelineSemaphoreCreateInfo.pNext = &semaphoreTypeInfo;
        timelineSemaphoreCreateInfo.flags = 0;

        if (VK_SUCCESS != vkCreateSemaphore(vk_state->device, &timelineSemaphoreCreateInfo, vk_state->vkAllocator, &vk_state->graphicsQueue.semaphore.handle) ||
            VK_SUCCESS != vkCreateSemaphore(vk_state->device, &timelineSemaphoreCreateInfo, vk_state->vkAllocator, &vk_state->transferQueue.semaphore.handle))
        {
            GRFATAL("Failed to create sync objects");
            return false;
        }

        vk_state->requestedQueueAcquisitionOperationsDarray = DarrayCreate(sizeof(VkDependencyInfo*), 10, vk_state->rendererAllocator, MEM_TAG_RENDERER_SUBSYS);

        GRTRACE("Successfully created vulkan queues");
    }

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
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (i32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            if ((VK_SUCCESS != vkCreateSemaphore(vk_state->device, &semaphoreCreateInfo, vk_state->vkAllocator, &vk_state->imageAvailableSemaphores[i])) ||
                (VK_SUCCESS != vkCreateSemaphore(vk_state->device, &semaphoreCreateInfo, vk_state->vkAllocator, &vk_state->renderFinishedSemaphores[i])))
            {
                GRFATAL("Failed to create sync objects");
                return false;
            }
        }

        vk_state->vertexUploadSemaphore.submitValue = 0;
        vk_state->indexUploadSemaphore.submitValue = 0;
        vk_state->imageUploadSemaphore.submitValue = 0;

        VkSemaphoreTypeCreateInfo semaphoreTypeInfo = {};
        semaphoreTypeInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        semaphoreTypeInfo.pNext = 0;
        semaphoreTypeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        semaphoreTypeInfo.initialValue = 0;

        VkSemaphoreCreateInfo timelineSemaphoreCreateInfo = {};
        timelineSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        timelineSemaphoreCreateInfo.pNext = &semaphoreTypeInfo;
        timelineSemaphoreCreateInfo.flags = 0;

        if (VK_SUCCESS != vkCreateSemaphore(vk_state->device, &timelineSemaphoreCreateInfo, vk_state->vkAllocator, &vk_state->vertexUploadSemaphore.handle) ||
            VK_SUCCESS != vkCreateSemaphore(vk_state->device, &timelineSemaphoreCreateInfo, vk_state->vkAllocator, &vk_state->indexUploadSemaphore.handle) ||
            VK_SUCCESS != vkCreateSemaphore(vk_state->device, &timelineSemaphoreCreateInfo, vk_state->vkAllocator, &vk_state->imageUploadSemaphore.handle))
        {
            GRFATAL("Failed to create sync objects");
            return false;
        }

        // max max frames in flight just needs to be higher than any sensible maxFramesInFlight value,
        // look at the wait for semaphores function at the start of the renderloop to understand why
        const u64 maxMaxFramesInFlight = 10;
        vk_state->frameSemaphore.submitValue = maxMaxFramesInFlight;
        semaphoreTypeInfo.initialValue = maxMaxFramesInFlight;

        if (VK_SUCCESS != vkCreateSemaphore(vk_state->device, &timelineSemaphoreCreateInfo, vk_state->vkAllocator, &vk_state->frameSemaphore.handle))
        {
            GRFATAL("Failed to create sync objects");
            return false;
        }

        GRTRACE("Vulkan sync objects created successfully");
    }

    // ============================================================================================================================================================
    // ======================== Creating the swapchain ============================================================================================================
    // ============================================================================================================================================================
    if (!CreateSwapchain(vk_state))
        return false;

    // ============================================================================================================================================================
    // ============================ Creating default texture ======================================================================================================
    // ============================================================================================================================================================
    {
#define DEFAULT_TEXTURE_SIZE 2

        u8 defaultTexturePixels[DEFAULT_TEXTURE_SIZE * DEFAULT_TEXTURE_SIZE * TEXTURE_CHANNELS] = {
            0, 0, 0, 255,     // pixel 0
            50, 50, 200, 255, // pixel 1
            0, 0, 0, 255,     // pixel 2
            50, 50, 200, 255, // pixel 3
        };

        vk_state->defaultTexture = TextureCreate(DEFAULT_TEXTURE_SIZE, DEFAULT_TEXTURE_SIZE, defaultTexturePixels);
    }

    Initialize2DRenderer();

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

    // Idling before shutting down the 2d renderer
    if (vk_state->device)
        vkDeviceWaitIdle(vk_state->device);

    Shutdown2DRenderer();

    // Idling again before destroying resources, because shutting down the 2d renderer might queue resources to be destroyed again
    if (vk_state->device)
        vkDeviceWaitIdle(vk_state->device);

    if (vk_state->requestedQueueAcquisitionOperationsDarray)
        DarrayDestroy(vk_state->requestedQueueAcquisitionOperationsDarray);

    // ============================================================================================================================================================
    // ============================ Destroying default texture ======================================================================================================
    // ============================================================================================================================================================
    if (vk_state->defaultTexture.internalState)
        TextureDestroy(vk_state->defaultTexture);

    if (vk_state->graphicsQueue.resourcesPendingDestructionDarray)
        TryDestroyResourcesPendingDestruction();

    // ============================================================================================================================================================
    // ====================== Destroying swapchain if it was created ==============================================================================================
    // ============================================================================================================================================================
    DestroySwapchain(vk_state);

    // ============================================================================================================================================================
    // ================================ Destroy sync objects if they were created =================================================================================
    // ============================================================================================================================================================
    for (i32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vk_state->imageAvailableSemaphores)
            vkDestroySemaphore(vk_state->device, vk_state->imageAvailableSemaphores[i], vk_state->vkAllocator);
        if (vk_state->renderFinishedSemaphores)
            vkDestroySemaphore(vk_state->device, vk_state->renderFinishedSemaphores[i], vk_state->vkAllocator);
    }

    if (vk_state->vertexUploadSemaphore.handle)
        vkDestroySemaphore(vk_state->device, vk_state->vertexUploadSemaphore.handle, vk_state->vkAllocator);
    if (vk_state->indexUploadSemaphore.handle)
        vkDestroySemaphore(vk_state->device, vk_state->indexUploadSemaphore.handle, vk_state->vkAllocator);
    if (vk_state->imageUploadSemaphore.handle)
        vkDestroySemaphore(vk_state->device, vk_state->imageUploadSemaphore.handle, vk_state->vkAllocator);
    if (vk_state->frameSemaphore.handle)
        vkDestroySemaphore(vk_state->device, vk_state->frameSemaphore.handle, vk_state->vkAllocator);

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
    if (vk_state->graphicsQueue.semaphore.handle)
        vkDestroySemaphore(vk_state->device, vk_state->graphicsQueue.semaphore.handle, vk_state->vkAllocator);
    if (vk_state->transferQueue.semaphore.handle)
        vkDestroySemaphore(vk_state->device, vk_state->transferQueue.semaphore.handle, vk_state->vkAllocator);

    if (vk_state->graphicsQueue.commandPool)
        vkDestroyCommandPool(vk_state->device, vk_state->graphicsQueue.commandPool, vk_state->vkAllocator);

    if (vk_state->transferQueue.commandPool)
        vkDestroyCommandPool(vk_state->device, vk_state->transferQueue.commandPool, vk_state->vkAllocator);

    if (vk_state->graphicsQueue.resourcesPendingDestructionDarray)
        DarrayDestroy(vk_state->graphicsQueue.resourcesPendingDestructionDarray);
    if (vk_state->transferQueue.resourcesPendingDestructionDarray)
        DarrayDestroy(vk_state->transferQueue.resourcesPendingDestructionDarray);

    // ============================================================================================================================================================
    // ===================== Destroying logical device if it was created ==========================================================================================
    // ============================================================================================================================================================
    if (vk_state->device)
        vkDestroyDevice(vk_state->device, vk_state->vkAllocator);
    if (vk_state->swapchainSupport.formats)
        Free(vk_state->rendererAllocator, vk_state->swapchainSupport.formats);
    if (vk_state->swapchainSupport.presentModes)
        Free(vk_state->rendererAllocator, vk_state->swapchainSupport.presentModes);

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

    DestroySwapchain(vk_state);

    CreateSwapchain(vk_state);

    vk_state->shouldRecreateSwapchain = false;
    GRINFO("Vulkan Swapchain resized");
}

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
    VkResult result = vkAcquireNextImageKHR(vk_state->device, vk_state->swapchain, UINT64_MAX, vk_state->imageAvailableSemaphores[vk_state->currentInFlightFrameIndex], VK_NULL_HANDLE, &vk_state->currentSwapchainImageIndex);

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

    // ====================================== Transition swapchain image to render attachment ======================================================
    {
        VkImageMemoryBarrier2 rendertargetTransitionImageBarrierInfo = {};
        rendertargetTransitionImageBarrierInfo.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        rendertargetTransitionImageBarrierInfo.pNext = nullptr;
        rendertargetTransitionImageBarrierInfo.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        rendertargetTransitionImageBarrierInfo.srcAccessMask = VK_ACCESS_2_NONE;
        rendertargetTransitionImageBarrierInfo.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        rendertargetTransitionImageBarrierInfo.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        rendertargetTransitionImageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        rendertargetTransitionImageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        rendertargetTransitionImageBarrierInfo.srcQueueFamilyIndex = 0;
        rendertargetTransitionImageBarrierInfo.dstQueueFamilyIndex = 0;
        rendertargetTransitionImageBarrierInfo.image = vk_state->swapchainImages[vk_state->currentSwapchainImageIndex];
        rendertargetTransitionImageBarrierInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rendertargetTransitionImageBarrierInfo.subresourceRange.baseMipLevel = 0;
        rendertargetTransitionImageBarrierInfo.subresourceRange.levelCount = 1;
        rendertargetTransitionImageBarrierInfo.subresourceRange.baseArrayLayer = 0;
        rendertargetTransitionImageBarrierInfo.subresourceRange.layerCount = 1;

        VkDependencyInfo rendertargetTransitionDependencyInfo = {};
        rendertargetTransitionDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        rendertargetTransitionDependencyInfo.pNext = nullptr;
        rendertargetTransitionDependencyInfo.dependencyFlags = 0;
        rendertargetTransitionDependencyInfo.memoryBarrierCount = 0;
        rendertargetTransitionDependencyInfo.bufferMemoryBarrierCount = 0;
        rendertargetTransitionDependencyInfo.imageMemoryBarrierCount = 1;
        rendertargetTransitionDependencyInfo.pImageMemoryBarriers = &rendertargetTransitionImageBarrierInfo;

        vkCmdPipelineBarrier2(currentCommandBuffer, &rendertargetTransitionDependencyInfo);
    }

    // ==================================== Begin renderpass ==============================================
    VkRenderingAttachmentInfo renderingAttachmentInfo = {};
    renderingAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    renderingAttachmentInfo.pNext = nullptr;
    renderingAttachmentInfo.imageView = vk_state->swapchainImageViews[vk_state->currentSwapchainImageIndex];
    renderingAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    renderingAttachmentInfo.resolveMode = 0;
    renderingAttachmentInfo.resolveImageView = nullptr;
    renderingAttachmentInfo.resolveImageLayout = 0;
    renderingAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    renderingAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    renderingAttachmentInfo.clearValue.color.float32[0] = 0;
    renderingAttachmentInfo.clearValue.color.float32[1] = 0;
    renderingAttachmentInfo.clearValue.color.float32[2] = 0;
    renderingAttachmentInfo.clearValue.color.float32[3] = 1.0f;

    VkRenderingInfo renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.pNext = nullptr;
    renderingInfo.flags = 0;
    renderingInfo.renderArea.offset.x = 0;
    renderingInfo.renderArea.offset.y = 0;
    renderingInfo.renderArea.extent = vk_state->swapchainExtent;
    renderingInfo.layerCount = 1;
    renderingInfo.viewMask = 0;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &renderingAttachmentInfo;
    renderingInfo.pDepthAttachment = nullptr;
    renderingInfo.pStencilAttachment = nullptr;

    vkCmdBeginRendering(currentCommandBuffer, &renderingInfo);

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
    Render2DScene();

    // ==================================== End renderpass =================================================
    vkCmdEndRendering(currentCommandBuffer);

    // ====================================== Transition swapchain image to present ready ======================================================
    {
        VkImageMemoryBarrier2 rendertargetTransitionImageBarrierInfo = {};
        rendertargetTransitionImageBarrierInfo.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        rendertargetTransitionImageBarrierInfo.pNext = nullptr;
        rendertargetTransitionImageBarrierInfo.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        rendertargetTransitionImageBarrierInfo.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        rendertargetTransitionImageBarrierInfo.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
        rendertargetTransitionImageBarrierInfo.dstAccessMask = VK_ACCESS_2_NONE;
        rendertargetTransitionImageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        rendertargetTransitionImageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        rendertargetTransitionImageBarrierInfo.srcQueueFamilyIndex = 0;
        rendertargetTransitionImageBarrierInfo.dstQueueFamilyIndex = 0;
        rendertargetTransitionImageBarrierInfo.image = vk_state->swapchainImages[vk_state->currentSwapchainImageIndex];
        rendertargetTransitionImageBarrierInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rendertargetTransitionImageBarrierInfo.subresourceRange.baseMipLevel = 0;
        rendertargetTransitionImageBarrierInfo.subresourceRange.levelCount = 1;
        rendertargetTransitionImageBarrierInfo.subresourceRange.baseArrayLayer = 0;
        rendertargetTransitionImageBarrierInfo.subresourceRange.layerCount = 1;

        VkDependencyInfo rendertargetTransitionDependencyInfo = {};
        rendertargetTransitionDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        rendertargetTransitionDependencyInfo.pNext = nullptr;
        rendertargetTransitionDependencyInfo.dependencyFlags = 0;
        rendertargetTransitionDependencyInfo.memoryBarrierCount = 0;
        rendertargetTransitionDependencyInfo.bufferMemoryBarrierCount = 0;
        rendertargetTransitionDependencyInfo.imageMemoryBarrierCount = 1;
        rendertargetTransitionDependencyInfo.pImageMemoryBarriers = &rendertargetTransitionImageBarrierInfo;

        vkCmdPipelineBarrier2(currentCommandBuffer, &rendertargetTransitionDependencyInfo);
    }

    // ================================= End command buffer recording ==================================================
    EndCommandBuffer(vk_state->graphicsCommandBuffers[vk_state->currentInFlightFrameIndex]);

    // =================================== Submitting command buffer ==============================================
    // With all the synchronization that that entails...
#define WAIT_SEMAPHORE_COUNT 4 // 1 swapchain image acquisition, 3 resourse upload waits
    VkSemaphoreSubmitInfo waitSemaphores[WAIT_SEMAPHORE_COUNT] = {};

    // Swapchain image acquisition semaphore
    waitSemaphores[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    waitSemaphores[0].pNext = nullptr;
    waitSemaphores[0].semaphore = vk_state->imageAvailableSemaphores[vk_state->currentInFlightFrameIndex];
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
    signalSemaphores[0].semaphore = vk_state->renderFinishedSemaphores[vk_state->currentInFlightFrameIndex];
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
    presentInfo.pWaitSemaphores = &vk_state->renderFinishedSemaphores[vk_state->currentInFlightFrameIndex];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vk_state->swapchain;
    presentInfo.pImageIndices = &vk_state->currentSwapchainImageIndex;
    presentInfo.pResults = nullptr;

    // When using mailbox present mode, vulkan will take care of skipping the presentation of this frame if another one is already finished
    vkQueuePresentKHR(vk_state->presentQueue, &presentInfo);

    vk_state->currentInFlightFrameIndex = (vk_state->currentInFlightFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

    return true;
}

static bool OnWindowResize(EventCode type, EventData data)
{
    vk_state->shouldRecreateSwapchain = true;
    return false;
}

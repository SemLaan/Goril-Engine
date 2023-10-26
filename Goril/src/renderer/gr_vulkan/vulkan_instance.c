#include "vulkan_instance.h"

#include "core/logger.h"
#include "vulkan_debug_tools.h"
#include "vulkan_utils.h"

bool CreateVulkanInstance(u32 requiredExtensionNameCount, const char** requiredExtensionNames, u32 requiredLayerNameCount, const char** requiredLayerNames)
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

        bool extensionsAvailable = CheckRequiredExtensions(requiredExtensionNameCount, requiredExtensionNames, availableExtensionCount, availableExtensionsDarray);

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

        bool layersAvailable = CheckRequiredLayers(requiredLayerNameCount, requiredLayerNames, availableLayerCount, availableLayersDarray);

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
        createInfo.enabledLayerCount = requiredLayerNameCount;
        createInfo.ppEnabledLayerNames = requiredLayerNames;
        createInfo.enabledExtensionCount = requiredExtensionNameCount;
        createInfo.ppEnabledExtensionNames = requiredExtensionNames;

        VkResult result = vkCreateInstance(&createInfo, vk_state->vkAllocator, &vk_state->instance);

        if (result != VK_SUCCESS)
        {
            GRFATAL("Failed to create Vulkan instance");
            return false;
        }
    }

    return true;
}

void DestroyVulkanInstance()
{
    if (vk_state->instance)
        vkDestroyInstance(vk_state->instance, vk_state->vkAllocator);
}

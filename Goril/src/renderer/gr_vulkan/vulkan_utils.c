#include "vulkan_utils.h"

// TODO: make a custom string thing with strncmp replacement so string.h doesn't have to be included
#include <string.h>


bool CheckRequiredExtensions(u32 requiredExtensionCount, const char** requiredExtensions, u32 availableExtensionCount, VkExtensionProperties* availableExtensions)
{
    u32 availableRequiredExtensions = 0;
    for (u32 i = 0; i < requiredExtensionCount; ++i)
    {
        for (u32 j = 0; j < availableExtensionCount; ++j)
        {
            if (0 == strncmp(requiredExtensions[i], availableExtensions[j].extensionName, VK_MAX_EXTENSION_NAME_SIZE))
            {
                availableRequiredExtensions++;
            }
        }
    }

    return availableRequiredExtensions == requiredExtensionCount;
}

bool CheckRequiredLayers(u32 requiredLayerCount, const char** requiredLayers, u32 availableLayerCount, VkLayerProperties* availableLayers)
{
    u32 availableRequiredLayers = 0;
    for (u32 i = 0; i < requiredLayerCount; ++i)
    {
        for (u32 j = 0; j < availableLayerCount; ++j)
        {
            if (0 == strncmp(requiredLayers[i], availableLayers[j].layerName, VK_MAX_EXTENSION_NAME_SIZE))
            {
                availableRequiredLayers++;
            }
        }
    }

    return availableRequiredLayers == requiredLayerCount;
}

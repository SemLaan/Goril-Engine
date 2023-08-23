#include "vulkan_debug_messenger.h"
#include "core/logger.h"

namespace GR
{
#ifndef GR_DIST
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		const char* type;

		switch (messageType)
		{
		case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
			type = "general    ";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
			type = "validation ";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
			type = "performance";
			break;
		}

		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			GRTRACE("VK Validation, {}: {}", type, pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			GRTRACE("VK Validation, {}: {}", type, pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			GRWARN("VK Validation, {}: {}", type, pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			GRERROR("VK Validation, {}: {}", type, pCallbackData->pMessage);
			break;
		}

		return VK_FALSE;
	}

	VkDebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo()
	{
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
		debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugMessengerCreateInfo.pNext = nullptr;
		debugMessengerCreateInfo.flags = 0;
		debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		//debugMessengerCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
		//debugMessengerCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
		debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		//debugMessengerCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
		debugMessengerCreateInfo.pfnUserCallback = debugCallback;
		debugMessengerCreateInfo.pUserData = nullptr;

		return debugMessengerCreateInfo;
	}

	b8 CreateDebugMessenger()
	{
		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk_state->instance, "vkCreateDebugUtilsMessengerEXT");
		VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = GetDebugMessengerCreateInfo();
		if (VK_SUCCESS != vkCreateDebugUtilsMessengerEXT(vk_state->instance, &messengerCreateInfo, vk_state->allocator, &vk_state->debugMessenger))
		{
			GRFATAL("Failed to create Vulkan debug utils messenger");
			return false;
		}

		return true;
	}

	void DestroyDebugMessenger()
	{
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk_state->instance, "vkDestroyDebugUtilsMessengerEXT");
		if (vk_state->debugMessenger)
			vkDestroyDebugUtilsMessengerEXT(vk_state->instance, vk_state->debugMessenger, vk_state->allocator);
	}
#endif // !GR_DIST
}
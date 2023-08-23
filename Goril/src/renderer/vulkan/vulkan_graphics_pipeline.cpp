#include "vulkan_graphics_pipeline.h"

#include "vulkan_shader_loader.h"
#include "../buffer.h"

namespace GR
{
	b8 CreateGraphicsPipeline(RendererState* state)
	{
		// Creating shader modules
		Darray<char> vert;
		Darray<char> frag;
		ReadFile("C:/Users/semla/Documents/Git_Repos/zelf_projecten/c++/Goril/Goril/src/renderer/shaders/vershader.spv", MEM_TAG_RENDERER_SUBSYS, &vert);
		ReadFile("C:/Users/semla/Documents/Git_Repos/zelf_projecten/c++/Goril/Goril/src/renderer/shaders/frshader.spv", MEM_TAG_RENDERER_SUBSYS, &frag);
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
		CreateShaderModule(state, vert, &vertShaderModule);
		CreateShaderModule(state, frag, &fragShaderModule);
		vert.Deinitialize();
		frag.Deinitialize();

		VkPipelineShaderStageCreateInfo vertStageCreateInfo{};
		vertStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertStageCreateInfo.pNext = nullptr;
		vertStageCreateInfo.flags = 0;
		vertStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertStageCreateInfo.module = vertShaderModule;
		vertStageCreateInfo.pName = "main";
		vertStageCreateInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo fragStageCreateInfo{};
		fragStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragStageCreateInfo.pNext = nullptr;
		fragStageCreateInfo.flags = 0;
		fragStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragStageCreateInfo.module = fragShaderModule;
		fragStageCreateInfo.pName = "main";
		fragStageCreateInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo shaderStagesCreateInfo[2] = { vertStageCreateInfo, fragStageCreateInfo };

		// Dynamic states
		constexpr u32 dynamicStateCount = 2;
		VkDynamicState dynamicStates[dynamicStateCount] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
		dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCreateInfo.pNext = nullptr;
		dynamicStateCreateInfo.flags = 0;
		dynamicStateCreateInfo.dynamicStateCount = dynamicStateCount;
		dynamicStateCreateInfo.pDynamicStates = dynamicStates;

		// Vertex input
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attributeDescriptions[2]{};
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.pNext = nullptr;
		vertexInputCreateInfo.flags = 0;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
		vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputCreateInfo.vertexAttributeDescriptionCount = 2;
		vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions;

		// Input assembler
		VkPipelineInputAssemblyStateCreateInfo inputAssemblerCreateInfo{};
		inputAssemblerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblerCreateInfo.pNext = nullptr;
		inputAssemblerCreateInfo.flags = 0;
		inputAssemblerCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblerCreateInfo.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
		viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.pNext = nullptr;
		viewportStateCreateInfo.flags = 0;
		viewportStateCreateInfo.viewportCount = 1;
		viewportStateCreateInfo.pViewports = nullptr; // dynamic state
		viewportStateCreateInfo.scissorCount = 1;
		viewportStateCreateInfo.pScissors = nullptr;  // dynamic state

		// Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
		rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerCreateInfo.pNext = nullptr;
		rasterizerCreateInfo.flags = 0;
		rasterizerCreateInfo.depthClampEnable = VK_FALSE;
		rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
		rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
		rasterizerCreateInfo.depthBiasClamp = 0.0f;
		rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;
		rasterizerCreateInfo.lineWidth = 1.0f;

		// Multisampling
		VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo{};
		multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisamplingCreateInfo.pNext = nullptr;
		multisamplingCreateInfo.flags = 0;
		multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
		multisamplingCreateInfo.minSampleShading = 1.0f;
		multisamplingCreateInfo.pSampleMask = nullptr;
		multisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
		multisamplingCreateInfo.alphaToOneEnable = VK_FALSE;

		// Depth/stencil
		/// TODO: add depth stencil state create info

		// Blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo blendStateCreateInfo{};
		blendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blendStateCreateInfo.pNext = nullptr;
		blendStateCreateInfo.flags = 0;
		blendStateCreateInfo.logicOpEnable = VK_FALSE;
		blendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
		blendStateCreateInfo.attachmentCount = 1;
		blendStateCreateInfo.pAttachments = &colorBlendAttachment;

		// Pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.flags = 0;
		pipelineLayoutCreateInfo.setLayoutCount = 0;
		pipelineLayoutCreateInfo.pSetLayouts = nullptr;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

		if (VK_SUCCESS != vkCreatePipelineLayout(state->device, &pipelineLayoutCreateInfo, state->allocator, &state->pipelineLayout))
		{
			GRFATAL("Vulkan pipeline layout creation failed");
			vkDestroyShaderModule(state->device, vertShaderModule, state->allocator);
			vkDestroyShaderModule(state->device, fragShaderModule, state->allocator);
			return false;
		}

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
		graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.pNext = nullptr;
		graphicsPipelineCreateInfo.flags = 0;
		graphicsPipelineCreateInfo.stageCount = 2;
		graphicsPipelineCreateInfo.pStages = shaderStagesCreateInfo;
		graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
		graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblerCreateInfo;
		graphicsPipelineCreateInfo.pTessellationState = nullptr;
		graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
		graphicsPipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
		graphicsPipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
		graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
		graphicsPipelineCreateInfo.pColorBlendState = &blendStateCreateInfo;
		graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
		graphicsPipelineCreateInfo.layout = state->pipelineLayout;
		graphicsPipelineCreateInfo.renderPass = state->renderpass;
		graphicsPipelineCreateInfo.subpass = 0;
		graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		graphicsPipelineCreateInfo.basePipelineIndex = -1;

		if (VK_SUCCESS != vkCreateGraphicsPipelines(state->device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, state->allocator, &state->graphicsPipeline))
		{
			GRFATAL("Vulkan graphics pipeline creation failed");
			vkDestroyShaderModule(state->device, vertShaderModule, state->allocator);
			vkDestroyShaderModule(state->device, fragShaderModule, state->allocator);
			return false;
		}

		vkDestroyShaderModule(state->device, vertShaderModule, state->allocator);
		vkDestroyShaderModule(state->device, fragShaderModule, state->allocator);

		return true;
	}

	void DestroyGraphicsPipeline(RendererState* state)
	{
		if (state->graphicsPipeline)
			vkDestroyPipeline(state->device, state->graphicsPipeline, state->allocator);
		if (state->pipelineLayout)
			vkDestroyPipelineLayout(state->device, state->pipelineLayout, state->allocator);
	}
}
// Implements both
#include "vulkan_2D_renderer.h"
#include "renderer/2D_renderer.h"

#include "core/asserts.h"
#include "renderer/renderer.h"
#include "vulkan_buffer.h"
#include "vulkan_shader_loader.h"
#include "containers/hashmap_u64.h"
#include "containers/darray.h"

// ============================================================================================================================================================
// ============================ vulkan 2d renderer.h ============================================================================================================
// ============================================================================================================================================================

typedef struct SpriteInstance
{
	mat4 model;
	u32 textureIndex;
} SpriteInstance;

#define COMBINED_IMAGE_SAMPLERS_ARRAY_SIZE 100

// State for the 2d renderer
typedef struct Renderer2DState
{
    SceneRenderData2D currentRenderData;            //
    GlobalUniformObject currentGlobalUBO;           //
    VertexBuffer quadVertexBuffer;                  //
    VertexBuffer instancedBuffer;                   //
    IndexBuffer quadIndexBuffer;                    //
    VkDescriptorSetLayout descriptorSetLayout;      //
    VkBuffer* uniformBuffersDarray;                 //
    VkDeviceMemory* uniformBuffersMemoryDarray;     //
    void** uniformBuffersMappedDarray;              //
    VkDescriptorPool uniformDescriptorPool;         //
    VkDescriptorSet* uniformDescriptorSetsDarray;   //
    VkPipelineLayout pipelineLayout;                //
    VkPipeline graphicsPipeline;                    //
    HashmapU64* textureMap;                         //
    Texture* textureDarray;                         //
} Renderer2DState;

static Renderer2DState* state2D = nullptr;

bool Initialize2DRenderer()
{
    state2D = Alloc(vk_state->rendererBumpAllocator, sizeof(*state2D), MEM_TAG_RENDERER_SUBSYS);

    // ============================================================================================================================================================
    // ======================== Creating graphics pipeline for the instanced quad shader ==========================================================================
    // ============================================================================================================================================================
    {
        // Creating shader modules
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
        CreateShaderModule("vershader.spv", vk_state, &vertShaderModule);
        CreateShaderModule("frshader.spv", vk_state, &fragShaderModule);

        VkDescriptorSetLayoutBinding uboLayoutBindings[2] = {};
        uboLayoutBindings[0].binding = 0;
        uboLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBindings[0].descriptorCount = 1;
        uboLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBindings[0].pImmutableSamplers = nullptr;

        uboLayoutBindings[1].binding = 1;
        uboLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        uboLayoutBindings[1].descriptorCount = COMBINED_IMAGE_SAMPLERS_ARRAY_SIZE;
        uboLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        uboLayoutBindings[1].pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.pNext = nullptr;
        descriptorSetLayoutCreateInfo.flags = 0;
        descriptorSetLayoutCreateInfo.bindingCount = 2;
        descriptorSetLayoutCreateInfo.pBindings = uboLayoutBindings;

        if (VK_SUCCESS != vkCreateDescriptorSetLayout(vk_state->device, &descriptorSetLayoutCreateInfo, vk_state->vkAllocator, &state2D->descriptorSetLayout))
        {
            GRFATAL("Vulkan descriptor set layout creation failed");
            vkDestroyShaderModule(vk_state->device, vertShaderModule, vk_state->vkAllocator);
            vkDestroyShaderModule(vk_state->device, fragShaderModule, vk_state->vkAllocator);
            return false;
        }

        VkPipelineShaderStageCreateInfo vertStageCreateInfo = {};
        vertStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertStageCreateInfo.pNext = nullptr;
        vertStageCreateInfo.flags = 0;
        vertStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertStageCreateInfo.module = vertShaderModule;
        vertStageCreateInfo.pName = "main";
        vertStageCreateInfo.pSpecializationInfo = nullptr;

        VkPipelineShaderStageCreateInfo fragStageCreateInfo = {};
        fragStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragStageCreateInfo.pNext = nullptr;
        fragStageCreateInfo.flags = 0;
        fragStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragStageCreateInfo.module = fragShaderModule;
        fragStageCreateInfo.pName = "main";
        fragStageCreateInfo.pSpecializationInfo = nullptr;

        VkPipelineShaderStageCreateInfo shaderStagesCreateInfo[2] = {vertStageCreateInfo, fragStageCreateInfo};

// Dynamic states
#define DYNAMIC_STATE_COUNT 2
        VkDynamicState dynamicStates[DYNAMIC_STATE_COUNT] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.pNext = nullptr;
        dynamicStateCreateInfo.flags = 0;
        dynamicStateCreateInfo.dynamicStateCount = DYNAMIC_STATE_COUNT;
        dynamicStateCreateInfo.pDynamicStates = dynamicStates;

        // Vertex input
        VkVertexInputBindingDescription vertexBindingDescription = {};
        vertexBindingDescription.binding = 0;
        vertexBindingDescription.stride = sizeof(Vertex);
        vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

#define VERTEX_ATTRIBUTE_COUNT 8
        VkVertexInputAttributeDescription attributeDescriptions[VERTEX_ATTRIBUTE_COUNT] = {};
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        // Vertex per instance input
        VkVertexInputBindingDescription instanceBindingDescription = {};
        instanceBindingDescription.binding = 1;
        instanceBindingDescription.stride = sizeof(SpriteInstance);
        instanceBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        // These nex four attributes are one matrix
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].binding = 1;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(SpriteInstance, model);
        //
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].binding = 1;
        attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[4].offset = offsetof(SpriteInstance, model) + sizeof(f32) * 4;
        //
        attributeDescriptions[5].location = 5;
        attributeDescriptions[5].binding = 1;
        attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[5].offset = offsetof(SpriteInstance, model) + sizeof(f32) * 8;
        //
        attributeDescriptions[6].location = 6;
        attributeDescriptions[6].binding = 1;
        attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[6].offset = offsetof(SpriteInstance, model) + sizeof(f32) * 12;

        attributeDescriptions[7].location = 7;
        attributeDescriptions[7].binding = 1;
        attributeDescriptions[7].format = VK_FORMAT_R32_UINT;
        attributeDescriptions[7].offset = offsetof(SpriteInstance, textureIndex);

        VkVertexInputBindingDescription bindingDescriptions[2] = {vertexBindingDescription, instanceBindingDescription};

        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
        vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputCreateInfo.pNext = nullptr;
        vertexInputCreateInfo.flags = 0;
        vertexInputCreateInfo.vertexBindingDescriptionCount = 2;
        vertexInputCreateInfo.pVertexBindingDescriptions = bindingDescriptions;
        vertexInputCreateInfo.vertexAttributeDescriptionCount = VERTEX_ATTRIBUTE_COUNT;
        vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions;

        // Input assembler
        VkPipelineInputAssemblyStateCreateInfo inputAssemblerCreateInfo = {};
        inputAssemblerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblerCreateInfo.pNext = nullptr;
        inputAssemblerCreateInfo.flags = 0;
        inputAssemblerCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblerCreateInfo.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.pNext = nullptr;
        viewportStateCreateInfo.flags = 0;
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.pViewports = nullptr; // dynamic state
        viewportStateCreateInfo.scissorCount = 1;
        viewportStateCreateInfo.pScissors = nullptr; // dynamic state

        // Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
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
        VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
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
        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo blendStateCreateInfo = {};
        blendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blendStateCreateInfo.pNext = nullptr;
        blendStateCreateInfo.flags = 0;
        blendStateCreateInfo.logicOpEnable = VK_FALSE;
        blendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
        blendStateCreateInfo.attachmentCount = 1;
        blendStateCreateInfo.pAttachments = &colorBlendAttachment;

        // Push constants
        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstantObject);

        // Pipeline layout
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.pNext = nullptr;
        pipelineLayoutCreateInfo.flags = 0;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &state2D->descriptorSetLayout;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

        if (VK_SUCCESS != vkCreatePipelineLayout(vk_state->device, &pipelineLayoutCreateInfo, vk_state->vkAllocator, &state2D->pipelineLayout))
        {
            GRFATAL("Vulkan pipeline layout creation failed");
            vkDestroyShaderModule(vk_state->device, vertShaderModule, vk_state->vkAllocator);
            vkDestroyShaderModule(vk_state->device, fragShaderModule, vk_state->vkAllocator);
            return false;
        }

        // Render target
        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {};
        pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipelineRenderingCreateInfo.pNext = nullptr;
        pipelineRenderingCreateInfo.viewMask = 0;
        pipelineRenderingCreateInfo.colorAttachmentCount = 1;
        pipelineRenderingCreateInfo.pColorAttachmentFormats = &vk_state->swapchainFormat;
        pipelineRenderingCreateInfo.depthAttachmentFormat = 0;
        pipelineRenderingCreateInfo.stencilAttachmentFormat = 0;

        VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
        graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineCreateInfo.pNext = &pipelineRenderingCreateInfo;
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
        graphicsPipelineCreateInfo.layout = state2D->pipelineLayout;
        graphicsPipelineCreateInfo.renderPass = nullptr;
        graphicsPipelineCreateInfo.subpass = 0;
        graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        graphicsPipelineCreateInfo.basePipelineIndex = -1;

        if (VK_SUCCESS != vkCreateGraphicsPipelines(vk_state->device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, vk_state->vkAllocator, &state2D->graphicsPipeline))
        {
            GRFATAL("Vulkan graphics pipeline creation failed");
            vkDestroyShaderModule(vk_state->device, vertShaderModule, vk_state->vkAllocator);
            vkDestroyShaderModule(vk_state->device, fragShaderModule, vk_state->vkAllocator);
            return false;
        }

        vkDestroyShaderModule(vk_state->device, vertShaderModule, vk_state->vkAllocator);
        vkDestroyShaderModule(vk_state->device, fragShaderModule, vk_state->vkAllocator);
    }

    // ============================================================================================================================================================
    // ======================== Creating uniform buffers for the instanced quad shader ============================================================================
    // ============================================================================================================================================================
    {
        VkDeviceSize uniformBufferSize = sizeof(GlobalUniformObject);

        state2D->uniformBuffersDarray = (VkBuffer*)DarrayCreateWithSize(sizeof(VkBuffer), MAX_FRAMES_IN_FLIGHT, vk_state->rendererAllocator, MEM_TAG_RENDERER_SUBSYS);
        state2D->uniformBuffersMemoryDarray = (VkDeviceMemory*)DarrayCreateWithSize(sizeof(VkDeviceMemory), MAX_FRAMES_IN_FLIGHT, vk_state->rendererAllocator, MEM_TAG_RENDERER_SUBSYS);
        state2D->uniformBuffersMappedDarray = (void**)DarrayCreateWithSize(sizeof(void*), MAX_FRAMES_IN_FLIGHT, vk_state->rendererAllocator, MEM_TAG_RENDERER_SUBSYS);

        for (i32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            CreateBuffer(uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &state2D->uniformBuffersDarray[i], &state2D->uniformBuffersMemoryDarray[i]);
            vkMapMemory(vk_state->device, state2D->uniformBuffersMemoryDarray[i], 0, uniformBufferSize, 0, &state2D->uniformBuffersMappedDarray[i]);
        }

        VkDescriptorPoolSize descriptorPoolSizes[2] = {};
        descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorPoolSizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;
        descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorPoolSizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT;

        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.pNext = nullptr;
        descriptorPoolCreateInfo.flags = 0;
        descriptorPoolCreateInfo.maxSets = MAX_FRAMES_IN_FLIGHT;
        descriptorPoolCreateInfo.poolSizeCount = 2;
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes;

        if (VK_SUCCESS != vkCreateDescriptorPool(vk_state->device, &descriptorPoolCreateInfo, vk_state->vkAllocator, &state2D->uniformDescriptorPool))
        {
            GRFATAL("Vulkan descriptor pool creation failed");
            return false;
        }

        VkDescriptorSetLayout descriptorSetLayouts[MAX_FRAMES_IN_FLIGHT];
        for (i32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            descriptorSetLayouts[i] = state2D->descriptorSetLayout;
        }

        VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
        descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocInfo.pNext = nullptr;
        descriptorSetAllocInfo.descriptorPool = state2D->uniformDescriptorPool;
        descriptorSetAllocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
        descriptorSetAllocInfo.pSetLayouts = descriptorSetLayouts;

        state2D->uniformDescriptorSetsDarray = (VkDescriptorSet*)DarrayCreateWithSize(sizeof(VkDescriptorSet), MAX_FRAMES_IN_FLIGHT, vk_state->rendererAllocator, MEM_TAG_RENDERER_SUBSYS);

        if (VK_SUCCESS != vkAllocateDescriptorSets(vk_state->device, &descriptorSetAllocInfo, state2D->uniformDescriptorSetsDarray))
        {
            GRFATAL("Vulkan descriptor set allocation failed");
            return false;
        }

        // =============================== Initialize descriptor sets ====================================================
        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            VkDescriptorBufferInfo descriptorBufferInfo = {};
            descriptorBufferInfo.buffer = state2D->uniformBuffersDarray[i];
            descriptorBufferInfo.offset = 0;
            descriptorBufferInfo.range = sizeof(GlobalUniformObject);

            VkDescriptorImageInfo descriptorImageInfos[COMBINED_IMAGE_SAMPLERS_ARRAY_SIZE];
            for (u32 i = 0; i < COMBINED_IMAGE_SAMPLERS_ARRAY_SIZE; ++i)
            {
                descriptorImageInfos[i].sampler = ((VulkanImage*)vk_state->defaultTexture.internalState)->sampler;
                descriptorImageInfos[i].imageView = ((VulkanImage*)vk_state->defaultTexture.internalState)->view;
                descriptorImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }

            VkWriteDescriptorSet descriptorWrites[2] = {};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].pNext = nullptr;
            descriptorWrites[0].dstSet = state2D->uniformDescriptorSetsDarray[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].pImageInfo = nullptr;
            descriptorWrites[0].pBufferInfo = &descriptorBufferInfo;
            descriptorWrites[0].pTexelBufferView = nullptr;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].pNext = nullptr;
            descriptorWrites[1].dstSet = state2D->uniformDescriptorSetsDarray[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorCount = COMBINED_IMAGE_SAMPLERS_ARRAY_SIZE;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].pImageInfo = descriptorImageInfos;
            descriptorWrites[1].pBufferInfo = nullptr;
            descriptorWrites[1].pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(vk_state->device, 2, descriptorWrites, 0, nullptr);
        }
    }

    // ============================================================================================================================================================
    // ======================== Creating the quad mesh ============================================================================================================
    // ============================================================================================================================================================
#define QUAD_VERT_COUNT 4
    Vertex quadVertices[QUAD_VERT_COUNT] =
        {
            {{0, 0, 0}, {0, 0, 0}, {0, 0}},
            {{0, 1, 0}, {0, 0, 0}, {0, 1}},
            {{1, 0, 0}, {0, 0, 0}, {1, 0}},
            {{1, 1, 0}, {0, 0, 0}, {1, 1}},
        };

    state2D->quadVertexBuffer = VertexBufferCreate(quadVertices, sizeof(quadVertices));

#define QUAD_INDEX_COUNT 6
    u32 quadIndices[QUAD_INDEX_COUNT] =
        {
            0, 1, 2,
            2, 1, 3};

    state2D->quadIndexBuffer = IndexBufferCreate(quadIndices, QUAD_INDEX_COUNT);
    state2D->instancedBuffer = VertexBufferCreate(nullptr, 100 * sizeof(SpriteInstance));

    const u32 mapBackingArraySize = 100;
    const u32 mapLinkedElementsSize = 10;
    state2D->textureMap = MapU64Create(vk_state->rendererBumpAllocator, MEM_TAG_RENDERER_SUBSYS, mapBackingArraySize, mapLinkedElementsSize, Hash6432Shift);

    state2D->textureDarray = DarrayCreate(sizeof(*state2D->textureDarray), mapBackingArraySize, vk_state->rendererAllocator, MEM_TAG_RENDERER_SUBSYS);

    GRTRACE("2D renderer initialized");

    return true;
}

void Shutdown2DRenderer()
{
    DarrayDestroy(state2D->textureDarray);
    MapU64Destroy(state2D->textureMap);

    // ============================================================================================================================================================
    // ======================== Destroying the quad mesh ==========================================================================================================
    // ============================================================================================================================================================
    IndexBufferDestroy(state2D->quadIndexBuffer);
    VertexBufferDestroy(state2D->instancedBuffer);
    VertexBufferDestroy(state2D->quadVertexBuffer);

    // ============================================================================================================================================================
    // ======================== Destroying uniform buffers for the instanced quad shader ==========================================================================
    // ============================================================================================================================================================

    // ============================================================================================================================================================
    // ======================== Destroying graphics pipeline for the instanced quad shader ========================================================================
    // ============================================================================================================================================================
    {
        if (state2D->graphicsPipeline)
            vkDestroyPipeline(vk_state->device, state2D->graphicsPipeline, vk_state->vkAllocator);
        if (state2D->pipelineLayout)
            vkDestroyPipelineLayout(vk_state->device, state2D->pipelineLayout, vk_state->vkAllocator);

        for (i32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            vkUnmapMemory(vk_state->device, state2D->uniformBuffersMemoryDarray[i]);
            vkDestroyBuffer(vk_state->device, state2D->uniformBuffersDarray[i], vk_state->vkAllocator);
            vkFreeMemory(vk_state->device, state2D->uniformBuffersMemoryDarray[i], vk_state->vkAllocator);
        }

        DarrayDestroy(state2D->uniformBuffersDarray);
        DarrayDestroy(state2D->uniformBuffersMappedDarray);
        DarrayDestroy(state2D->uniformBuffersMemoryDarray);
        DarrayDestroy(state2D->uniformDescriptorSetsDarray);

        if (state2D->uniformDescriptorPool)
            vkDestroyDescriptorPool(vk_state->device, state2D->uniformDescriptorPool, vk_state->vkAllocator);

        if (state2D->descriptorSetLayout)
            vkDestroyDescriptorSetLayout(vk_state->device, state2D->descriptorSetLayout, vk_state->vkAllocator);
    }

    Free(vk_state->rendererBumpAllocator, state2D);
}

void Preprocess2DSceneData()
{
    // ========================== Preprocessing camera ================================
    state2D->currentGlobalUBO.projView = state2D->currentRenderData.camera;

    // =========================== preprocessing quads =================================
    u32 instanceCount = DarrayGetSize(state2D->currentRenderData.spriteRenderInfoDarray);
    GRASSERT_DEBUG(instanceCount > 0);

    DarraySetSize(state2D->textureDarray, 0);
    // TODO: clear map

    for (u32 i = 0; i < instanceCount; ++i)
    {

    }

    // TODO: bind textures
    // TODO: set uniforms (textures and camera)

    SpriteInstance* instanceData = Alloc(vk_state->rendererAllocator, sizeof(*instanceData) * instanceCount, MEM_TAG_RENDERER_SUBSYS);

    for (u32 i = 0; i < instanceCount; ++i)
    {
        instanceData[i].model = state2D->currentRenderData.spriteRenderInfoDarray[i].model;
        instanceData[i].textureIndex = 0; // TODO: fill instanced buffer with texture indices
    }

    VertexBufferUpdate(state2D->instancedBuffer, instanceData, instanceCount * sizeof(*instanceData));

    Free(vk_state->rendererAllocator, instanceData);

    // =============================== Updating descriptor sets ==================================
    MemCopy(state2D->uniformBuffersMappedDarray[vk_state->currentInFlightFrameIndex], &state2D->currentGlobalUBO, sizeof(GlobalUniformObject));

    {
        VkDescriptorBufferInfo descriptorBufferInfo = {};
        descriptorBufferInfo.buffer = state2D->uniformBuffersDarray[vk_state->currentInFlightFrameIndex];
        descriptorBufferInfo.offset = 0;
        descriptorBufferInfo.range = sizeof(GlobalUniformObject);

        VkDescriptorImageInfo descriptorImageInfos[COMBINED_IMAGE_SAMPLERS_ARRAY_SIZE];
        for (u32 i = 0; i < COMBINED_IMAGE_SAMPLERS_ARRAY_SIZE; ++i)
        {
            descriptorImageInfos[i].sampler = ((VulkanImage*)vk_state->defaultTexture.internalState)->sampler;// TODO: 
            descriptorImageInfos[i].imageView = ((VulkanImage*)vk_state->defaultTexture.internalState)->view;
            descriptorImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        VkWriteDescriptorSet descriptorWrites[2] = {};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].pNext = nullptr;
        descriptorWrites[0].dstSet = state2D->uniformDescriptorSetsDarray[vk_state->currentInFlightFrameIndex];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].pImageInfo = nullptr;
        descriptorWrites[0].pBufferInfo = &descriptorBufferInfo;
        descriptorWrites[0].pTexelBufferView = nullptr;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].pNext = nullptr;
        descriptorWrites[1].dstSet = state2D->uniformDescriptorSetsDarray[vk_state->currentInFlightFrameIndex];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorCount = COMBINED_IMAGE_SAMPLERS_ARRAY_SIZE;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].pImageInfo = descriptorImageInfos;
        descriptorWrites[1].pBufferInfo = nullptr;
        descriptorWrites[1].pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(vk_state->device, 2, descriptorWrites, 0, nullptr);
    }
}

void Render2DScene()
{
    VkCommandBuffer currentCommandBuffer = vk_state->graphicsCommandBuffers[vk_state->currentInFlightFrameIndex].handle;

    // Binding global descriptor set
    vkCmdBindDescriptorSets(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state2D->pipelineLayout, 0, 1, &state2D->uniformDescriptorSetsDarray[vk_state->currentInFlightFrameIndex], 0, nullptr);

    vkCmdBindPipeline(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state2D->graphicsPipeline);

    u32 instanceCount = DarrayGetSize(state2D->currentRenderData.spriteRenderInfoDarray);

    VulkanVertexBuffer* quadBuffer = state2D->quadVertexBuffer.internalState;
    VulkanIndexBuffer* indexBuffer = state2D->quadIndexBuffer.internalState;
    VulkanVertexBuffer* instancedBuffer = state2D->instancedBuffer.internalState;

    VkBuffer vertexBuffers[2] = {quadBuffer->handle, instancedBuffer->handle};

    VkDeviceSize offsets[2] = {0, 0};
    vkCmdBindVertexBuffers(currentCommandBuffer, 0, 2, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(currentCommandBuffer, indexBuffer->handle, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(currentCommandBuffer, indexBuffer->indexCount, instanceCount, 0, 0, 0);

    DarrayDestroy(state2D->currentRenderData.spriteRenderInfoDarray);
}

// ============================================================================================================================================================
// ============================ 2D renderer.h ============================================================================================================
// ============================================================================================================================================================
void Submit2DScene(SceneRenderData2D sceneData)
{
    state2D->currentRenderData = sceneData;
}

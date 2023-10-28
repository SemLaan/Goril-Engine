// Implements both
#include "vulkan_2D_renderer.h"
#include "renderer/2D_renderer.h"

#include "renderer/renderer.h"
#include "vulkan_graphics_pipeline.h"
#include "core/asserts.h"


// ============================================================================================================================================================
// ============================ vulkan 2d renderer.h ============================================================================================================
// ============================================================================================================================================================

// State for the 2d renderer
typedef struct Renderer2DState
{
    SceneRenderData2D currentRenderData;
    GlobalUniformObject currentGlobalUBO;
    VertexBuffer quadVertexBuffer;
    VertexBuffer instancedBuffer;
    IndexBuffer quadIndexBuffer;
} Renderer2DState;

static Renderer2DState* state2D = nullptr;

bool Initialize2DRenderer()
{
    // ============================================================================================================================================================
    // ======================== Creating graphics pipeline for the instanced quad shader ==========================================================================
    // ============================================================================================================================================================
    if (!CreateGraphicsPipeline())
        return false;

    // ============================================================================================================================================================
    // ======================== Creating the quad mesh ============================================================================================================
    // ============================================================================================================================================================
    state2D = Alloc(vk_state->rendererBumpAllocator, sizeof(*state2D), MEM_TAG_RENDERER_SUBSYS);

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

    return true;
}

void Shutdown2DRenderer()
{
    // ============================================================================================================================================================
    // ======================== Destroying the quad mesh ==========================================================================================================
    // ============================================================================================================================================================
    IndexBufferDestroy(state2D->quadIndexBuffer);
    VertexBufferDestroy(state2D->instancedBuffer);
    VertexBufferDestroy(state2D->quadVertexBuffer);

    // ============================================================================================================================================================
    // ======================== Destroying graphics pipeline for the instanced quad shader ========================================================================
    // ============================================================================================================================================================
    DestroyGraphicsPipeline();

    Free(vk_state->rendererBumpAllocator, state2D);
}

void Preprocess2DSceneData()
{
    // ========================== Preprocessing camera ================================
    state2D->currentGlobalUBO.projView = state2D->currentRenderData.camera;

    // =========================== preprocessing quads =================================
    u32 spriteCount = DarrayGetSize(state2D->currentRenderData.spriteRenderInfoDarray);
    GRASSERT(spriteCount > 0);

    // TODO: bind textures
    // TODO: set uniforms (textures and camera)

    SpriteInstance* instanceData = Alloc(vk_state->rendererAllocator, sizeof(*instanceData) * spriteCount, MEM_TAG_RENDERER_SUBSYS);

    for (u32 i = 0; i < spriteCount; ++i)
    {
        instanceData[i].model = state2D->currentRenderData.spriteRenderInfoDarray[i].model;
        instanceData[i].textureIndex = 0; // TODO: fill instanced buffer with texture indices
    }

    VertexBufferUpdate(state2D->instancedBuffer, instanceData, spriteCount * sizeof(*instanceData));

    Free(vk_state->rendererAllocator, instanceData);

    // =============================== Updating descriptor sets ==================================
    MemCopy(vk_state->uniformBuffersMappedDarray[vk_state->currentInFlightFrameIndex], &state2D->currentGlobalUBO, sizeof(GlobalUniformObject));
    UpdateDescriptorSets(vk_state->currentInFlightFrameIndex, (VulkanImage*)state2D->currentRenderData.spriteRenderInfoDarray[0].texture.internalState); // TODO: make gfx pipelines configurable
}

void Render2DScene()
{
    u32 spriteCount = DarrayGetSize(state2D->currentRenderData.spriteRenderInfoDarray);

    VkCommandBuffer currentCommandBuffer = vk_state->graphicsCommandBuffers[vk_state->currentInFlightFrameIndex].handle;

    VulkanVertexBuffer* quadBuffer = state2D->quadVertexBuffer.internalState;
    VulkanIndexBuffer* indexBuffer = state2D->quadIndexBuffer.internalState;
    VulkanVertexBuffer* instancedBuffer = state2D->instancedBuffer.internalState;

    VkBuffer vertexBuffers[2] = {quadBuffer->handle, instancedBuffer->handle};

    VkDeviceSize offsets[2] = {0, 0};
    vkCmdBindVertexBuffers(currentCommandBuffer, 0, 2, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(currentCommandBuffer, indexBuffer->handle, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(currentCommandBuffer, indexBuffer->indexCount, spriteCount, 0, 0, 0);

    DarrayDestroy(state2D->currentRenderData.spriteRenderInfoDarray);
}


// ============================================================================================================================================================
// ============================ 2D renderer.h ============================================================================================================
// ============================================================================================================================================================
void Submit2DScene(SceneRenderData2D sceneData)
{
    state2D->currentRenderData = sceneData;
}


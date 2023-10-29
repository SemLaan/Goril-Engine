#include "game.h"
#include <core/event.h>
#include <core/input.h>
#include <core/logger.h>
#include <core/meminc.h>
#include <core/timer.h>
#include <platform/platform.h>
#include <renderer/renderer.h>
#include <renderer/2D_renderer.h>
#include <renderer/texture.h>

Allocator* gameAllocator;
GameState* gamestate = nullptr;

bool Init()
{
    // =========================== Allocating game memory ============================================================
    CreateFreelistAllocator("Game allocator", GetGlobalAllocator(), KiB * 5, &gameAllocator);
    gamestate = Alloc(gameAllocator, sizeof(*gamestate), MEM_TAG_GAME);
    PRINT_MEMORY_STATS();

    // =========================== Setting up the game camera ==========================================================
    vec2i windowSize = GetPlatformWindowSize();
    float windowAspectRatio = windowSize.x / (float)windowSize.y;
    i32 orthoWidth = 20;
    gamestate->proj = mat4_orthographic(-orthoWidth / 2, orthoWidth / 2, -orthoWidth / 2 / windowAspectRatio, orthoWidth / 2 / windowAspectRatio, 0.1f, 1000.0f);
    gamestate->view = mat4_identity();
    gamestate->camPosition = (vec3){0, 0, 10};
    // Create the rotation matrix
    gamestate->camRotation = mat4_rotate_xyz((vec3){0, PI, 0});

    // =========================== Creating the texture =============================================================
    #define textureSize 100
    u8 texturePixels[textureSize * textureSize * TEXTURE_CHANNELS];

    for (u32 i = 0; i < textureSize * textureSize; ++i)
    {
        u32 pixelIndex = i * TEXTURE_CHANNELS;

        texturePixels[pixelIndex + 0] = i;
        texturePixels[pixelIndex + 1] = 7;
        texturePixels[pixelIndex + 2] = 49;
        texturePixels[pixelIndex + 3] = 255;
    }

    gamestate->texture = TextureCreate(textureSize, textureSize, texturePixels);


    return true;
}

bool Shutdown()
{
    TextureDestroy(gamestate->texture);

    Free(gameAllocator, gamestate);

    DestroyFreelistAllocator(gameAllocator);

    return true;
}

bool Update()
{
    // Updating camera position based on player movement
    vec3 rightVector = {gamestate->camRotation.values[0][0], gamestate->camRotation.values[1][0], gamestate->camRotation.values[2][0]};
    vec3 frameMovement = {};
    if (GetKeyDown(KEY_A))
        frameMovement = vec3_add_vec3(frameMovement, rightVector);
    if (GetKeyDown(KEY_D))
        frameMovement = vec3_min_vec3(frameMovement, rightVector);
    if (GetKeyDown(KEY_S))
        frameMovement.y -= 1;
    if (GetKeyDown(KEY_W))
        frameMovement.y += 1;
    gamestate->camPosition = vec3_add_vec3(gamestate->camPosition, vec3_div_float(frameMovement, 300.f));

    // Updating the view matrix based on new camera position
    mat4 translate = mat4_translate(gamestate->camPosition);
    gamestate->view = mat4_mul_mat4(gamestate->camRotation, translate);

    return true;
}

bool Render()
{
    SceneRenderData2D sceneData = {};
    sceneData.camera = mat4_mul_mat4(gamestate->proj, gamestate->view);
    sceneData.spriteRenderInfoDarray = DarrayCreate(sizeof(*sceneData.spriteRenderInfoDarray), 1, GetGlobalAllocator(), MEM_TAG_GAME);

    SpriteRenderInfo sprite = {};
    sprite.model = mat4_identity();
    sprite.texture = gamestate->texture;

    sceneData.spriteRenderInfoDarray = DarrayPushback(sceneData.spriteRenderInfoDarray, &sprite);

    sprite.model = mat4_translate((vec3){2, 2, 0});
    sprite.texture = gamestate->texture;
    sceneData.spriteRenderInfoDarray = DarrayPushback(sceneData.spriteRenderInfoDarray, &sprite);

    Submit2DScene(sceneData);

    return true;
}


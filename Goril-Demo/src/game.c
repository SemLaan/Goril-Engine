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
    CreateFreelistAllocator("Game allocator", GetGlobalAllocator(), KiB * 5, &gameAllocator);

    gamestate = Alloc(gameAllocator, sizeof(*gamestate), MEM_TAG_GAME);

    PRINT_MEMORY_STATS();


    vec2i windowSize = GetPlatformWindowSize();
    float windowAspectRatio = windowSize.x / (float)windowSize.y;
    i32 orthoWidth = 20;
    gamestate->perspective = mat4_perspective(45.0f, windowAspectRatio, 0.1f, 1000.0f);
    gamestate->orthographic = mat4_orthographic(-orthoWidth / 2, orthoWidth / 2, -orthoWidth / 2 / windowAspectRatio, orthoWidth / 2 / windowAspectRatio, 0.1f, 1000.0f);
    gamestate->proj = gamestate->perspective;
    gamestate->view = mat4_identity();
    gamestate->camPosition = (vec3){0, -3, 0};
    gamestate->camRotation = (vec3){0, 0, 0};

    const u32 textureSize = 100;
    u8* texturePixels = Alloc(GetGlobalAllocator(), textureSize * textureSize * TEXTURE_CHANNELS, MEM_TAG_GAME);

    for (u32 i = 0; i < textureSize * textureSize; ++i)
    {
        u32 pixelIndex = i * TEXTURE_CHANNELS;

        texturePixels[pixelIndex + 0] = i;
        texturePixels[pixelIndex + 1] = 7;
        texturePixels[pixelIndex + 2] = 49;
        texturePixels[pixelIndex + 3] = 255;
    }

    gamestate->texture = TextureCreate(textureSize, textureSize, texturePixels);

    Free(GetGlobalAllocator(), texturePixels);

    #define texture2Size 2

    u8 texture2Pixels[texture2Size * texture2Size * TEXTURE_CHANNELS] = 
    {
        255, 0, 0, 255,
        0, 255, 0, 255,
        0, 0, 255, 255,
        255, 255, 255, 255,
    };

    gamestate->texture2 = TextureCreate(texture2Size, texture2Size, texture2Pixels);

    gamestate->mouseEnabled = false;
    gamestate->perspectiveEnabled = true;

    return true;
}

bool Shutdown()
{
    TextureDestroy(gamestate->texture);
    TextureDestroy(gamestate->texture2);

    Free(gameAllocator, gamestate);

    DestroyFreelistAllocator(gameAllocator);

    return true;
}

bool Update()
{
    const f32 mouseMoveSpeed = 3500;

    if (gamestate->mouseEnabled)
    {
        gamestate->camRotation.y -= GetMouseDistanceFromCenter().x / mouseMoveSpeed;
        gamestate->camRotation.x += GetMouseDistanceFromCenter().y / mouseMoveSpeed;
    }
    if (gamestate->camRotation.x > 1.5f)
        gamestate->camRotation.x = 1.5f;
    if (gamestate->camRotation.x < -1.5f)
        gamestate->camRotation.x = -1.5f;

    // Create the rotation matrix
    mat4 rotation = mat4_rotate_xyz(gamestate->camRotation);

    vec3 forwardVector = {-rotation.values[0][2], -rotation.values[1][2], -rotation.values[2][2]};
    vec3 rightVector = {rotation.values[0][0], rotation.values[1][0], rotation.values[2][0]};

    vec3 frameMovement = {};

    if (GetKeyDown(KEY_A))
        frameMovement = vec3_add_vec3(frameMovement, rightVector);
    if (GetKeyDown(KEY_D))
        frameMovement = vec3_min_vec3(frameMovement, rightVector);
    if (GetKeyDown(KEY_S))
        frameMovement = vec3_add_vec3(frameMovement, forwardVector);
    if (GetKeyDown(KEY_W))
        frameMovement = vec3_min_vec3(frameMovement, forwardVector);
    if (GetKeyDown(KEY_SHIFT))
        frameMovement.y -= 1;
    if (GetKeyDown(KEY_SPACE))
        frameMovement.y += 1;
    gamestate->camPosition = vec3_add_vec3(gamestate->camPosition, vec3_div_float(frameMovement, 300.f));

    mat4 translate = mat4_translate(gamestate->camPosition);

    gamestate->view = mat4_mul_mat4(rotation, translate);

    if (GetButtonDown(BUTTON_LEFTMOUSEBTN) && !GetButtonDownPrevious(BUTTON_LEFTMOUSEBTN))
    {
        gamestate->mouseEnabled = !gamestate->mouseEnabled;
        InputSetMouseCentered(gamestate->mouseEnabled);
    }

    if (GetButtonDown(BUTTON_RIGHTMOUSEBTN) && !GetButtonDownPrevious(BUTTON_RIGHTMOUSEBTN))
    {
        gamestate->perspectiveEnabled = !gamestate->perspectiveEnabled;
        if (gamestate->perspectiveEnabled)
            gamestate->proj = gamestate->perspective;
        else
            gamestate->proj = gamestate->orthographic;
    }

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

    sprite.model = mat4_translate((vec3){2, 2, 2});
    sprite.texture = gamestate->texture2;
    sceneData.spriteRenderInfoDarray = DarrayPushback(sceneData.spriteRenderInfoDarray, &sprite);

    Submit2DScene(sceneData);

    return true;
}


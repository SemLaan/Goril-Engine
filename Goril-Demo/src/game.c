#include "game.h"
#include <core/event.h>
#include <core/input.h>
#include <core/logger.h>
#include <core/meminc.h>
#include <core/timer.h>
#include <platform/platform.h>
#include <renderer/renderer.h>
#include <renderer/texture.h>

Allocator gameAllocator;
GameState* gamestate = nullptr;

bool Init()
{
    CreateFreelistAllocator("Game allocator", GetGlobalAllocator(), KiB * 5, &gameAllocator);

    gamestate = Alloc(&gameAllocator, sizeof(*gamestate), MEM_TAG_GAME);

    PRINT_MEMORY_STATS();

#define VERTEX_COUNT 8
    Vertex vertices[VERTEX_COUNT] =
        {
            {{-1, -1, 1}, {1.f, 0.f, 0.f}, {0.0f, 0.0f}},
            {{1, -1, 1}, {0.f, 1.f, 0.f}, {1.0f, 0.0f}},
            {{-1, 1, 1}, {0.f, 0.f, 1.f}, {0.0f, 1.0f}},
            {{1, 1, 1}, {1.f, 1.f, 1.f}, {1.0f, 1.0f}},
            {{-1, -1, -1}, {1.f, 1.f, 1.f}, {0.0f, 0.0f}},
            {{1, -1, -1}, {1.f, 1.f, 1.f}, {1.0f, 0.0f}},
            {{-1, 1, -1}, {1.f, 1.f, 1.f}, {0.0f, 1.0f}},
            {{1, 1, -1}, {1.f, 1.f, 1.f}, {1.0f, 1.0f}}};

    gamestate->vertexBuffer = VertexBufferCreate(nullptr, sizeof(vertices));

#define INDEX_COUNT (6 * 6)
    u32 indices[INDEX_COUNT] = {
        // Top
        7, 6, 2,
        2, 3, 7,
        // Bottom
        0, 4, 5,
        5, 1, 0,
        // Left
        0, 2, 6,
        6, 4, 0,
        // Right
        7, 3, 1,
        1, 5, 7,
        // Front
        3, 2, 0,
        0, 1, 3,
        // Back
        4, 6, 7,
        7, 5, 4};
    gamestate->indexBuffer = IndexBufferCreate(indices, INDEX_COUNT);

    vec2i windowSize = GetPlatformWindowSize();
    float windowAspectRatio = windowSize.x / (float)windowSize.y;
    i32 orthoWidth = 20;
    gamestate->perspective = mat4_perspective(45.0f, windowAspectRatio, 0.1f, 1000.0f);
    gamestate->orthographic = mat4_orthographic(-orthoWidth / 2, orthoWidth / 2, -orthoWidth / 2 / windowAspectRatio, orthoWidth / 2 / windowAspectRatio, 0.1f, 1000.0f);
    gamestate->proj = gamestate->perspective;
    gamestate->view = mat4_identity();
    gamestate->camPosition = (vec3){0, -3, 0};
    gamestate->camRotation = (vec3){0, 0, 0};

    u32 textureWidth = 100;
    u32 textureHeight = 100;
    u8* texturePixels = Alloc(GetGlobalAllocator(), textureWidth * textureHeight * TEXTURE_CHANNELS, MEM_TAG_GAME);

    for (u32 i = 0; i < textureWidth * textureHeight; ++i)
    {
        u32 pixelIndex = i * TEXTURE_CHANNELS;

        texturePixels[pixelIndex + 0] = i;
        texturePixels[pixelIndex + 1] = 7;
        texturePixels[pixelIndex + 2] = 49;
        texturePixels[pixelIndex + 3] = 255;
    }

    gamestate->texture = TextureCreate(textureWidth, textureHeight, texturePixels);

    Free(GetGlobalAllocator(), texturePixels);

    gamestate->mouseEnabled = false;
    gamestate->perspectiveEnabled = true;
    gamestate->meshOneActive = true;

    return true;
}

bool Update()
{
    f32 mouseMoveSpeed = 3500;

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

    Vertex vertices1[VERTEX_COUNT] = {
        {{-1, -1, 1}, {1.f, 0.f, 0.f}, {0.0f, 0.0f}},
        {{1, -1, 1}, {0.f, 1.f, 0.f}, {1.0f, 0.0f}},
        {{-1, 1, 1}, {0.f, 0.f, 1.f}, {0.0f, 1.0f}},
        {{1, 1, 1}, {1.f, 1.f, 1.f}, {1.0f, 1.0f}},
        {{-1, -1, -1}, {1.f, 1.f, 1.f}, {0.0f, 0.0f}},
        {{1, -1, -1}, {1.f, 1.f, 1.f}, {1.0f, 0.0f}},
        {{-1, 1, -1}, {1.f, 1.f, 1.f}, {0.0f, 1.0f}},
        {{1, 1, -1}, {1.f, 1.f, 1.f}, {1.0f, 1.0f}}};

    Vertex vertices2[VERTEX_COUNT] = {
        {{-1, -1, 1}, {1.f, 0.f, 0.f}, {0.0f, 0.0f}},
        {{1, -1, 1}, {0.f, 1.f, 0.f}, {1.0f, 0.0f}},
        {{-1, 1, 1}, {0.f, 0.f, 1.f}, {0.0f, 1.0f}},
        {{1, 1, 1}, {1.f, 1.f, 1.f}, {1.0f, 1.0f}},
        {{-1, -1, -1}, {1.f, 1.f, 1.f}, {0.0f, 0.0f}},
        {{2, -1, -1}, {1.f, 1.f, 1.f}, {1.0f, 0.0f}},
        {{-1, 1, -1}, {1.f, 1.f, 1.f}, {0.0f, 1.0f}},
        {{1, 1, -1}, {1.f, 1.f, 1.f}, {1.0f, 1.0f}}};

    gamestate->meshOneActive += g_deltaTime;

    if (gamestate->meshOneActive > 5)
    {
        gamestate->meshOneActive = 0;
        VertexBufferUpdate(gamestate->vertexBuffer, vertices1, sizeof(vertices1));
    }
    else if (gamestate->meshOneActive > 2.5f && gamestate->meshOneActive < 2.6f)
    {
        VertexBufferUpdate(gamestate->vertexBuffer, vertices2, sizeof(vertices2));
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
    sceneData.spriteRenderInfoDarray = DarrayPushback(sceneData.spriteRenderInfoDarray, &sprite);

    Submit2DScene(sceneData);

    return true;
}

bool Shutdown()
{
    IndexBufferDestroy(gamestate->indexBuffer);
    VertexBufferDestroy(gamestate->vertexBuffer);
    TextureDestroy(gamestate->texture);

    Free(&gameAllocator, gamestate);

    DestroyFreelistAllocator(gameAllocator);

    return true;
}

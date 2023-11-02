#include "game.h"
#include <core/event.h>
#include <core/input.h>
#include <core/logger.h>
#include <core/meminc.h>
#include <core/timer.h>
#include <platform/platform.h>
#include <renderer/2D_renderer.h>
#include <renderer/renderer.h>
#include <renderer/texture.h>

Allocator* gameAllocator;
GameState* gamestate = nullptr;


bool Init()
{
    // =========================== Allocating game memory ============================================================
    CreateFreelistAllocator("Game allocator", GetGlobalAllocator(), KiB * 5, &gameAllocator);
    gamestate = Alloc(gameAllocator, sizeof(*gamestate), MEM_TAG_GAME);
    ZeroMem(gamestate, sizeof(*gamestate));
    PRINT_MEMORY_STATS();

    // =========================== Setting up the game camera ==========================================================
    vec2i windowSize = GetPlatformWindowSize();
    f32 windowAspectRatio = windowSize.x / (f32)windowSize.y;
    f32 orthoWidth = 20.f;
    //gamestate->camera = CameraCreateOrthographic(-orthoWidth / 2.f, orthoWidth / 2.f, -orthoWidth / 2.f / windowAspectRatio, orthoWidth / 2.f / windowAspectRatio, 0.1f, 100.0f);
    gamestate->camera = CameraCreateOrthographic(0, orthoWidth, 0, orthoWidth / windowAspectRatio, 0.1f, 100.0f);
    CameraSetPosition(&gamestate->camera, (vec3){0, 0, 10});

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

    gamestate->buttons[0].size = (vec2){5.f, 5.f};
    gamestate->buttons[0].position = (vec2){2.5f, 0.f};

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
    vec3 frameMovement = {};
    if (GetKeyDown(KEY_A))
        frameMovement.x -= 1;
    if (GetKeyDown(KEY_D))
        frameMovement.x += 1;
    if (GetKeyDown(KEY_S))
        frameMovement.y -= 1;
    if (GetKeyDown(KEY_W))
        frameMovement.y += 1;
    CameraSetPosition(&gamestate->camera, vec3_add_vec3(CameraGetPosition(&gamestate->camera), vec3_div_float(frameMovement, 300.f)));


    if (GetButtonDown(BUTTON_LEFTMOUSEBTN) && !GetButtonDownPrevious(BUTTON_LEFTMOUSEBTN))
    {
        vec2 mouseScreenPos = (vec2){GetMousePos().x, GetMousePos().y};

        vec4 mouseWorldPos = CameraScreenToWorldSpace(&gamestate->camera, mouseScreenPos);
        GRDEBUG("world: x: %.2f, y: %.2f, z: %.2f", mouseWorldPos.x, mouseWorldPos.y, mouseWorldPos.z);

        vec4 test = mat4_mul_vec4(CameraGetProjectionView(&gamestate->camera), mouseWorldPos);
        GRDEBUG("clip:  x: %.2f, y: %.2f, z: %.2f", test.x, test.y, test.z);

        mouseWorldPos.z = 0;

        for (u32 i = 0; i < 2; ++i)
        {
            f32 left, right, top, bottom;
            left = gamestate->buttons[i].position.x;
            right = gamestate->buttons[i].position.x + gamestate->buttons[i].size.x;
            bottom = gamestate->buttons[i].position.y;
            top = gamestate->buttons[i].position.y + gamestate->buttons[i].size.y;
            if (mouseWorldPos.x > left && mouseWorldPos.x < right && mouseWorldPos.y > bottom && mouseWorldPos.y < top)
            {
                GRDEBUG("beef");
            }
        }
    }

    // Submitting the scene for rendering
    SceneRenderData2D sceneData = {};
    sceneData.camera = CameraGetProjectionView(&gamestate->camera);
    sceneData.spriteRenderInfoDarray = DarrayCreate(sizeof(*sceneData.spriteRenderInfoDarray), 1, GetGlobalAllocator(), MEM_TAG_GAME);

    SpriteRenderInfo sprite = {};
    sprite.model = mat4_mul_mat4(mat4_2Dtranslate(gamestate->buttons[0].position), mat4_2Dscale(gamestate->buttons[0].size));
    sprite.texture = gamestate->texture;

    sceneData.spriteRenderInfoDarray = DarrayPushback(sceneData.spriteRenderInfoDarray, &sprite);

    sprite.model = mat4_3Dtranslate((vec3){0.f, 5.0f, 0});
    sprite.texture = gamestate->texture;
    sceneData.spriteRenderInfoDarray = DarrayPushback(sceneData.spriteRenderInfoDarray, &sprite);

    Submit2DScene(sceneData);

    return true;
}


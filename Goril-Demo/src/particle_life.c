#include "particle_life.h"

#include <renderer/2D_renderer.h>
#include <renderer/texture.h>
#include <math/color_utils.h>

#define PARTICLE_TEXTURE_SIZE 100
#define COLOR_COUNT 5
#define PARTICLE_COUNT 500

typedef struct State
{
    Allocator* allocator;
    Camera* camera;
    Texture circleTextures[COLOR_COUNT];
    vec2 particlePositions[PARTICLE_COUNT];
    vec2 particleVelocities[PARTICLE_COUNT];
} State;

State* state = nullptr;

//
//

void ParticlelifeStart(Allocator* allocator, Camera* camera)
{
    state = AlignedAlloc(allocator, sizeof(*state), 64/*cache line*/, MEM_TAG_GAME);
    ZeroMem(state, sizeof(*state));
    state->allocator = allocator;
    state->camera = camera;

    // Generate textures
    for (u32 j = 0; j < COLOR_COUNT; ++j)
    {
        u8 texturePixels[PARTICLE_TEXTURE_SIZE * PARTICLE_TEXTURE_SIZE * TEXTURE_CHANNELS];

        f32 hue = j * (360 / COLOR_COUNT);
        vec3 colorHSB = (vec3){hue, 100.f, 100.f};
        vec3i colorRGB = HSB_to_RGB(colorHSB.x, colorHSB.y, colorHSB.z);

        for (u32 i = 0; i < PARTICLE_TEXTURE_SIZE * PARTICLE_TEXTURE_SIZE; ++i)
        {
            u32 pixelIndex = i * TEXTURE_CHANNELS;
            f32 x = i % PARTICLE_TEXTURE_SIZE;
            f32 y = (i - x) / PARTICLE_TEXTURE_SIZE;
            x /= (PARTICLE_TEXTURE_SIZE-1) * 0.5;
            y /= (PARTICLE_TEXTURE_SIZE-1) * 0.5;
            x -= 1;
            y -= 1;

            texturePixels[pixelIndex + 0] = colorRGB.x;
            texturePixels[pixelIndex + 1] = colorRGB.y;
            texturePixels[pixelIndex + 2] = colorRGB.z;
            if (x * x + y * y > 1)
                texturePixels[pixelIndex + 3] = 0;
            else
                texturePixels[pixelIndex + 3] = 255;
        }

        state->circleTextures[j] = TextureCreate(PARTICLE_TEXTURE_SIZE, PARTICLE_TEXTURE_SIZE, texturePixels);
    }
}

void ParticlelifeShutdown()
{
    // Destroy textures
    for (u32 i = 0; i < COLOR_COUNT; ++i)
    {
        TextureDestroy(state->circleTextures[i]);
    }

    Free(state->allocator, state);
}

static void UpdateVelocities()
{
}

static void UpdatePositions()
{
}

static void Render()
{
    // Submitting the scene for rendering
    SceneRenderData2D sceneData = {};
    sceneData.camera = CameraGetProjectionView(state->camera);
    sceneData.spriteRenderInfoDarray = DarrayCreate(sizeof(*sceneData.spriteRenderInfoDarray), PARTICLE_COUNT, state->allocator, MEM_TAG_GAME);

    SpriteRenderInfo sprite = {};
    sprite.model = mat4_identity();
    sprite.texture = state->circleTextures[0];

    mat4 particleScale = mat4_2Dscale((vec2){0.2f, 0.2f}); 
    for (u32 i = 0; i < PARTICLE_COUNT; ++i)
    {
        sprite.model = mat4_mul_mat4(mat4_2Dtranslate((vec2){i * 0.2f, 0}), particleScale);
        sprite.texture = state->circleTextures[i % COLOR_COUNT];
        sceneData.spriteRenderInfoDarray = DarrayPushback(sceneData.spriteRenderInfoDarray, &sprite);
    }

    Submit2DScene(sceneData);
}

void ParticlelifeUpdate()
{
    UpdateVelocities();
    UpdatePositions();
    Render();
}

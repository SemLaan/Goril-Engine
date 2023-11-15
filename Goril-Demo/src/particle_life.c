#include "particle_life.h"

#include <renderer/2D_renderer.h>
#include <renderer/texture.h>
#include <math/color_utils.h>
#include <platform/platform.h>
#include <math/random_utils.h>

#define PARTICLE_TEXTURE_SIZE 30
#define COLOR_COUNT 5
#define PARTICLE_COUNT 3000
#define PARTICLE_RADIUS 0.05f
#define MAX_DISTANCE (PARTICLE_RADIUS * 13.1f)
#define MIN_DISTANCE_PERCENT 0.3f
#define FRICTION_COEF 0.9f
#define DELTA_TIME 0.001f

typedef struct State
{
    Allocator* allocator;
    Camera* camera;
    u32 randomSeed;
    i32 simWidth;
    i32 simHeight;
    f32 attractionMatrix[COLOR_COUNT][COLOR_COUNT];
    Texture circleTextures[COLOR_COUNT];
    vec2 particlePositions[PARTICLE_COUNT];
    vec2 particleVelocities[PARTICLE_COUNT];
} State;

State* state = nullptr;

//
//

void ParticlelifeStart(Allocator* allocator, Camera* camera, i32 simWidth, i32 simHeight)
{
    state = AlignedAlloc(allocator, sizeof(*state), 64/*cache line*/, MEM_TAG_GAME);
    ZeroMem(state, sizeof(*state));
    state->allocator = allocator;
    state->camera = camera;
    state->randomSeed = 36;
    state->simWidth = simWidth;
    state->simHeight = simHeight;

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

    // Generate random positions
    for (u32 i = 0; i < PARTICLE_COUNT; ++i)
    {
        state->particlePositions[i].x = RandomFloat(&state->randomSeed) * state->simWidth;
        state->particlePositions[i].y = RandomFloat(&state->randomSeed) * state->simHeight;
    }

    // Generate random attraction values for all color pairs
    for (u32 i = 0; i < COLOR_COUNT; ++i)
    {
        for (u32 j = 0; j < COLOR_COUNT; ++j)
        {
            state->attractionMatrix[i][j] = (RandomFloat(&state->randomSeed) * 2) - 1;
        }
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
    for (u32 i = 0; i < PARTICLE_COUNT; ++i)
    {
        vec2 totalForce = {};
        for (u32 j = 0; j < PARTICLE_COUNT; ++j)
        {
            if (i == j) continue;
            vec2 hypothenuse = vec2_sub_vec2(state->particlePositions[j], state->particlePositions[i]);
            f32 magnitude = vec2_magnitude(hypothenuse);
            magnitude /= MAX_DISTANCE;
            vec2 direction = (vec2){hypothenuse.x / magnitude, hypothenuse.y / magnitude};
            if (magnitude < MIN_DISTANCE_PERCENT)
            {
                f32 force = magnitude / MIN_DISTANCE_PERCENT - 1;
                totalForce = vec2_add_vec2(totalForce, vec2_mul_f32(direction, force));
            }
        }

        //totalForce = vec2_mul_f32(totalForce, MAX_DISTANCE);
        state->particleVelocities[i] = vec2_add_vec2(vec2_mul_f32(state->particleVelocities[i], FRICTION_COEF), vec2_mul_f32(totalForce, DELTA_TIME));
    }
}

static void UpdatePositions()
{
    for (u32 i = 0; i < PARTICLE_COUNT; ++i)
    {
        state->particlePositions[i] = vec2_add_vec2(state->particlePositions[i], state->particleVelocities[i]);
        if (state->particlePositions[i].x > state->simWidth)
            state->particlePositions[i].x -= state->simWidth;
        if (state->particlePositions[i].x < 0)
            state->particlePositions[i].x += state->simWidth;
        if (state->particlePositions[i].y > state->simHeight)
            state->particlePositions[i].y -= state->simHeight;
        if (state->particlePositions[i].y < 0)
            state->particlePositions[i].y += state->simHeight;
    }
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

    mat4 particleScale = mat4_mul_mat4(mat4_2Dtranslate((vec2){-PARTICLE_RADIUS/2, -PARTICLE_RADIUS/2}), mat4_2Dscale((vec2){PARTICLE_RADIUS, PARTICLE_RADIUS})); 
    for (u32 i = 0; i < PARTICLE_COUNT; ++i)
    {
        sprite.model = mat4_mul_mat4(mat4_2Dtranslate(state->particlePositions[i]), particleScale);
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

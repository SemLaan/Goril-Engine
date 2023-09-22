#include "game.h"
#include <core/gr_memory.h>
#include <core/logger.h>
#include <core/input.h>
#include <core/event.h>
#include <renderer/renderer.h>
#include <platform/platform.h>


GameState* gamestate = nullptr;

bool Init()
{
	gamestate = (GameState*)Alloc(GetGlobalAllocator(), sizeof(*gamestate), MEM_TAG_GAME);

    PrintMemoryStats();

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
		{{1, 1, -1}, {1.f, 1.f, 1.f}, {1.0f, 1.0f}}
	};

	gamestate->vertexBuffer = CreateVertexBuffer(vertices, sizeof(Vertex) * VERTEX_COUNT);

	#define INDEX_COUNT (6 * 6)
	u32 indices[INDEX_COUNT] = {
		//Top
		7, 6, 2,
		2, 3, 7,
		//Bottom
		0, 4, 5,
		5, 1, 0,
		//Left
		0, 2, 6,
		6, 4, 0,
		//Right
		7, 3, 1,
		1, 5, 7,
		//Front
		3, 2, 0,
		0, 1, 3,
		//Back
		4, 6, 7,
		7, 5, 4
	};
	gamestate->indexBuffer = CreateIndexBuffer(indices, INDEX_COUNT);

	vec2i windowSize = GetPlatformWindowSize();
	gamestate->proj = mat4_perspective(45.0f, windowSize.x / (float)windowSize.y, 0.1f, 1000.0f);
	//gamestate->proj = mat4_identity();
	gamestate->view = mat4_identity();
	gamestate->camPosition = (vec3){ 0, -3, 0 };
	gamestate->camRotation = (vec3){ 0, 0, 0 };

	u32 textureWidth = 100;
	u32 textureHeight = 100;
	u8* texturePixels = (u8*)Alloc(GetGlobalAllocator(), textureWidth * textureHeight * TEXTURE_CHANNELS, MEM_TAG_GAME);

	for (u32 i = 0; i < textureWidth * textureHeight; ++i)
	{
		u32 pixelIndex = i * TEXTURE_CHANNELS;

		texturePixels[pixelIndex + 0] = i;
		texturePixels[pixelIndex + 1] = 7;
		texturePixels[pixelIndex + 2] = 49;
		texturePixels[pixelIndex + 3] = 255;
	}

	gamestate->texture = CreateTexture(textureWidth, textureHeight, texturePixels);

	Free(GetGlobalAllocator(), texturePixels);

    return true;
}

bool Update()
{
	f32 mouseMoveSpeed = 3500;

	gamestate->camRotation.x -= GetMouseDistanceFromCenter().x / mouseMoveSpeed;
	gamestate->camRotation.y += GetMouseDistanceFromCenter().y / mouseMoveSpeed;
	if (gamestate->camRotation.y > 1.5f)
		gamestate->camRotation.y = 1.5f;
	if (gamestate->camRotation.y < -1.5f)
		gamestate->camRotation.y = -1.5f;

	// Create the rotation matrix
	mat4 R_combined = mat4_rotate_xyz((vec3){ gamestate->camRotation.y, gamestate->camRotation.x, gamestate->camRotation.z });

	vec3 forwardVector = { -R_combined.values[0][2], -R_combined.values[1][2], -R_combined.values[2][2] };
	vec3 rightVector = { R_combined.values[0][0], R_combined.values[1][0], R_combined.values[2][0] };

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

	gamestate->view = mat4_mul_mat4(R_combined, translate);

	if (GetButtonDown(BUTTON_LEFTMOUSEBTN) && !GetButtonDownPrevious(BUTTON_LEFTMOUSEBTN))
		ToggleMouseCentered();

    return true;
}

bool Render()
{
	GlobalUniformObject ubo = {};
	ubo.projView = mat4_mul_mat4(gamestate->proj, gamestate->view);
	UpdateGlobalUniforms(&ubo, gamestate->texture);

	for (u32 i = 0; i < 3; ++i)
	{
		PushConstantObject pushValues = {};
		pushValues.model = mat4_scale((vec3){ 2, 2, 2 });
		pushValues.model = mat4_mul_mat4(pushValues.model, mat4_rotate_x((i+0.1f)/0.4f));
		pushValues.model = mat4_mul_mat4(pushValues.model, mat4_translate((vec3){ (f32)i * 3, 0, 0 }));

		DrawIndexed(gamestate->vertexBuffer, gamestate->indexBuffer, &pushValues);
	}
	
    return true;
}

bool Shutdown()
{
	DestroyIndexBuffer(gamestate->indexBuffer);
	DestroyVertexBuffer(gamestate->vertexBuffer);
	DestroyTexture(gamestate->texture);
	
	Free(GetGlobalAllocator(), gamestate);

    return true;
}

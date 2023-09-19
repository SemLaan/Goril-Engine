#include "game.h"
#include <core/gr_memory.h>
#include <core/logger.h>
#include <core/input.h>
#include <core/event.h>
#include <renderer/renderer.h>
#include <glm/gtc/matrix_transform.hpp>
#include <core/platform.h>


GameState* gamestate = nullptr;

bool Init()
{
	gamestate = (GameState*)Alloc(GetGlobalAllocator(), sizeof(*gamestate), MEM_TAG_GAME);

    PrintMemoryStats();

	const u32 vertexCount = 8;
	Vertex vertices[vertexCount] =
	{
		Vertex({-1, -1, 1}, {1.f, 0.f, 0.f}, {0.0f, 0.0f}),
		Vertex({1, -1, 1}, {0.f, 1.f, 0.f}, {1.0f, 0.0f}),
		Vertex({-1, 1, 1}, {0.f, 0.f, 1.f}, {0.0f, 1.0f}),
		Vertex({ {1, 1, 1}, {1.f, 1.f, 1.f}, {1.0f, 1.0f} }),
		Vertex({ {-1, -1, -1}, {1.f, 1.f, 1.f}, {0.0f, 0.0f} }),
		Vertex({ {1, -1, -1}, {1.f, 1.f, 1.f}, {1.0f, 0.0f} }),
		Vertex({ {-1, 1, -1}, {1.f, 1.f, 1.f}, {0.0f, 1.0f} }),
		Vertex({ {1, 1, -1}, {1.f, 1.f, 1.f}, {1.0f, 1.0f} })
	};

	gamestate->vertexBuffer = CreateVertexBuffer(vertices, sizeof(Vertex) * vertexCount);

	const u32 indexCount = 6 * 6;
	u32 indices[indexCount] = { 
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
	gamestate->indexBuffer = CreateIndexBuffer(indices, indexCount);

	glm::ivec2 windowSize = GetPlatformWindowSize();
	gamestate->proj = glm::perspective(glm::radians(45.0f), windowSize.x / (float)windowSize.y, 0.1f, 1000.0f);
	gamestate->view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	gamestate->camPosition = glm::vec3(0, -3, 0);
	gamestate->camRotation = glm::vec3(0);

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

	gamestate->camRotation.x += GetMouseDistanceFromCenter().x / mouseMoveSpeed;
	gamestate->camRotation.y += GetMouseDistanceFromCenter().y / mouseMoveSpeed;
	if (gamestate->camRotation.y > 1.5f)
		gamestate->camRotation.y = 1.5f;
	if (gamestate->camRotation.y < -1.5f)
		gamestate->camRotation.y = -1.5f;

	// Create individual rotation matrices
	glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), gamestate->camRotation.x, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), gamestate->camRotation.y, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 Rz = glm::rotate(glm::mat4(1.0f), gamestate->camRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

	// Combine the rotation matrices
	glm::mat4 R_combined = Rz * Ry * Rx;

	glm::vec3 forwardVector = -glm::vec3(R_combined[0][2], R_combined[1][2], R_combined[2][2]);
	glm::vec3 rightVector = glm::vec3(R_combined[0][0], R_combined[1][0], R_combined[2][0]);

	glm::vec3 frameMovement = glm::vec3(0);

	if (GetKeyDown(KEY_A))
		frameMovement += rightVector;
	if (GetKeyDown(KEY_D))
		frameMovement -= rightVector;
	if (GetKeyDown(KEY_S))
		frameMovement += forwardVector;
	if (GetKeyDown(KEY_W))
		frameMovement -= forwardVector;
	if (GetKeyDown(KEY_SHIFT))
		frameMovement.y += 1;
	if (GetKeyDown(KEY_SPACE))
		frameMovement.y -= 1;
	gamestate->camPosition += frameMovement / 300.f;

	glm::mat4 translate = glm::translate(glm::mat4(1.0f), gamestate->camPosition);

	gamestate->view = R_combined * translate;

	if (GetButtonDown(BUTTON_LEFTMOUSEBTN) && !GetButtonDownPrevious(BUTTON_LEFTMOUSEBTN))
		ToggleMouseCentered();

    return true;
}

bool Render()
{
	GlobalUniformObject ubo{};
	ubo.projView = gamestate->proj * gamestate->view;
	UpdateGlobalUniforms(&ubo, gamestate->texture);

	for (u32 i = 0; i < 3; ++i)
	{
		PushConstantObject pushValues{};
		pushValues.model = glm::scale(glm::mat4(1), glm::vec3(2, 2, 2));
		pushValues.model = glm::rotate(pushValues.model, (i+0.1f)/0.4f, glm::vec3(1, 0, 0));
		pushValues.model = glm::translate(pushValues.model, glm::vec3(i * 3, 0, 0));

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

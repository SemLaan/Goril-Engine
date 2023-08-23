#include "game.h"
#include <core/gr_memory.h>
#include <core/logger.h>
#include <core/input.h>
#include <core/event.h>
#include <renderer/renderer.h>
#include <glm/gtc/matrix_transform.hpp>
#include <core/platform.h>

using namespace GR;

b8 Game::Init()
{
    PrintMemoryStats();

	Darray<Vertex> vertices = Darray<Vertex>();
	vertices.Initialize(MEM_TAG_GAME);
	vertices.Pushback({ {-0.5f, -0.5f}, {1.f, 0.f, 0.f} });
	vertices.Pushback({ {0.5f, -0.5f}, {0.f, 1.f, 0.f} });
	vertices.Pushback({ {0.5f, 0.5f}, {0.f, 0.f, 1.f} });
	vertices.Pushback({ {-0.5f, 0.5f}, {1.f, 1.f, 1.f} });
	vertexBuffer = CreateVertexBuffer(vertices.GetRawElements(), sizeof(Vertex) * vertices.Size());
	vertices.Deinitialize();

	constexpr u32 indexCount = 6;
	u32 indices[indexCount] = { 0, 1, 2, 2, 3, 0 };
	indexBuffer = CreateIndexBuffer(indices, indexCount);

	glm::ivec2 windowSize = GetPlatformWindowSize();
	proj = glm::perspective(glm::radians(45.0f), windowSize.x / (float)windowSize.y, 0.1f, 10.0f);
	view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	camPosition = glm::vec3(-1);
	camRotation = glm::vec3(0);

    return true;
}

b8 Game::Update()
{
	f32 mouseMoveSpeed = 1600;
	//GRDEBUG("{}, {}", GetMouseDistanceFromCenter().x, GetMouseDistanceFromCenter().y);
	camRotation.x += GetMouseDistanceFromCenter().x / mouseMoveSpeed;
	camRotation.y += GetMouseDistanceFromCenter().y / mouseMoveSpeed;
	if (camRotation.y > 1.5f)
		camRotation.y = 1.5f;
	if (camRotation.y < -1.5f)
		camRotation.y = -1.5f;

	// Create individual rotation matrices
	glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), camRotation.x, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), camRotation.y, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 Rz = glm::rotate(glm::mat4(1.0f), camRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

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
	camPosition += frameMovement / 1000.f;

	glm::mat4 translate = glm::translate(glm::mat4(1.0f), camPosition);

	view = R_combined * translate;

	if (GetButtonDown(BUTTON_LEFTMOUSEBTN) && !GetButtonDownPrevious(BUTTON_LEFTMOUSEBTN))
		ToggleMouseCentered();

    return true;
}

b8 Game::Render()
{
	glm::mat4 model = glm::mat4(1);

	GlobalUniformObject ubo{};
	ubo.projView = proj * view * model;

	UpdateGlobalUniforms(&ubo);
	DrawIndexed(vertexBuffer, indexBuffer);
    return true;
}

b8 Game::Shutdown()
{
	DestroyIndexBuffer(indexBuffer);
	DestroyVertexBuffer(vertexBuffer);
    return true;
}

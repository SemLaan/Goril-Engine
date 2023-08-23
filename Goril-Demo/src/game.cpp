#include "game.h"
#include <core/gr_memory.h>
#include <core/logger.h>
//#include <core/input.h>
//#include <core/event.h>
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
    return true;
}

b8 Game::Update()
{
    return true;
}

b8 Game::Render()
{
	u32 width, height;
	GetPlatformWindowSize(&width, &height);
	glm::mat4 model = glm::rotate(glm::mat4(1.0f), 1.2f * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);

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

#include "game.h"
#include <core/gr_memory.h>
#include <core/logger.h>
//#include <core/input.h>
//#include <core/event.h>
#include <rendering/renderer.h>

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
	DrawIndexed(vertexBuffer, indexBuffer);
    return true;
}

b8 Game::Shutdown()
{
	DestroyIndexBuffer(indexBuffer);
	DestroyVertexBuffer(vertexBuffer);
    return true;
}

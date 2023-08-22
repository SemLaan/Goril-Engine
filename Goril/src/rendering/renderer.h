#pragma once
#include "defines.h"
#include "buffer.h"

namespace GR
{

	b8 InitializeRenderer();
	void ShutdownRenderer();

	void RecreateSwapchain();

	b8 BeginFrame();
	void EndFrame();

	void DrawIndexed(VertexBuffer vertexBuffer, IndexBuffer indexBuffer);
}
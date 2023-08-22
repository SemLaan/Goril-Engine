#pragma once
#include "defines.h"
#include "buffer.h"

namespace GR
{

	b8 InitializeRenderer();
	void ShutdownRenderer();

	void WaitForGPUIdle();
	void RecreateSwapchain();

	b8 BeginFrame();
	void EndFrame();

	GRAPI void DrawIndexed(VertexBuffer vertexBuffer, IndexBuffer indexBuffer);
}
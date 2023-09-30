#pragma once
#include "defines.h"
#include "buffer.h"

bool InitializeRenderer();
void ShutdownRenderer();

void RecreateSwapchain();

bool RenderFrame();

// Submit 2d scene takes ownership over sceneData
void Submit2DScene(SceneRenderData2D sceneData);

void DrawIndexed(VertexBuffer vertexBuffer, IndexBuffer indexBuffer, PushConstantObject* pPushConstantValues);

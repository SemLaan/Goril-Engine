#pragma once
#include "defines.h"
#include "buffer.h"

bool InitializeRenderer();
void ShutdownRenderer();

void RecreateSwapchain();

bool BeginFrame();
void EndFrame();

// Submit 2d scene takes ownership over sceneData
void Submit2DScene(SceneRenderData2D sceneData);

void UpdateGlobalUniforms(GlobalUniformObject* globalUniformObject, Texture texture);
void DrawIndexed(VertexBuffer vertexBuffer, IndexBuffer indexBuffer, PushConstantObject* pPushConstantValues);

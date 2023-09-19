#pragma once
#include "defines.h"
#include "buffer.h"


bool InitializeRenderer();
void ShutdownRenderer();

void RecreateSwapchain();

bool BeginFrame();
void EndFrame();

GRAPI void UpdateGlobalUniforms(GlobalUniformObject* globalUniformObject, Texture texture);
GRAPI void DrawIndexed(VertexBuffer vertexBuffer, IndexBuffer indexBuffer, PushConstantObject* pPushConstantValues);

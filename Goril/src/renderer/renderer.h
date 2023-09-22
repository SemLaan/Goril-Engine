#pragma once
#include "defines.h"
#include "buffer.h"


bool InitializeRenderer();
void ShutdownRenderer();

void RecreateSwapchain();

bool BeginFrame();
void EndFrame();

void UpdateGlobalUniforms(GlobalUniformObject* globalUniformObject, Texture texture);
void DrawIndexed(VertexBuffer vertexBuffer, IndexBuffer indexBuffer, PushConstantObject* pPushConstantValues);

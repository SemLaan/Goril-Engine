#pragma once
#include "defines.h"
#include "containers/darray.h"
#include "core/meminc.h"
#include "renderer_types.h"


VertexBuffer VertexBufferCreate(void* vertices, size_t size);
void VertexBufferUpdate(VertexBuffer clientBuffer, void* vertices, u64 size);
void VertexBufferDestroy(VertexBuffer clientBuffer);

/// <summary>
/// Creates an index buffer
/// </summary>
/// <param name="indices">Array of indices, type needs to be u32</param>
/// <param name="indexCount"></param>
/// <returns></returns>
IndexBuffer IndexBufferCreate(u32* indices, size_t indexCount);
void IndexBufferDestroy(IndexBuffer clientBuffer);

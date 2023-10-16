#pragma once

#include "defines.h"


// Copies memory from source to destination, also works if source and destination overlap
void MemCopy(void* destination, const void* source, size_t size);

void ZeroMem(void* block, u64 size);

void SetMem(void* block, i32 value, u64 size);

bool CompareMemory(void* a, void* b, u64 size);
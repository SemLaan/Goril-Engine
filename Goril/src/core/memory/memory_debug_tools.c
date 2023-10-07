#include "memory_debug_tools.h"

#ifndef GR_DIST

#include "allocator_frontends.h"
#include "core/asserts.h"

static const char* memTagToText[MAX_MEMORY_TAGS] = {
	"ALLOCATOR STATE    ",
	"SUB ARENA          ",
	"MEMORY SUBSYS      ",
	"LOGGING SUBSYS     ",
	"PLATFORM SUBSYS    ",
	"EVENT SUBSYS       ",
	"RENDERER SUBSYS    ",
	"INPUT SUBSYS       ",
	"GAME               ",
	"TEST               ",
	"DARRAY             ",
	"VERTEX BUFFER      ",
	"INDEX BUFFER       ",
	"TEXTURE            ",
	"HASHMAP            ",
};


static Allocator memoryDebugAllocator;


void StartMemoryDebugSubsys()
{
    const u64 memoryDebugArenaSize = MiB * 10;
    u64 allocatorStateSize;

    if (!CreateGlobalAllocator(memoryDebugArenaSize, &memoryDebugAllocator, &allocatorStateSize))
		GRASSERT_MSG(false, "Creating memory debug allocator failed");
}

void ShutdownMemoryDebugSubsys()
{
    DestroyGlobalAllocator(memoryDebugAllocator);
}


void* DebugAlignedAlloc(Allocator* allocator, u64 size, u32 alignment)
{
    return allocator->BackendAlloc(allocator, size, alignment);
}

void* DebugRealloc(Allocator* allocator, void* block, u64 newSize)
{
    return allocator->BackendRealloc(allocator, block, newSize);
}

void DebugFree(Allocator* allocator, void* block)
{
    allocator->BackendFree(allocator, block);
}


#endif

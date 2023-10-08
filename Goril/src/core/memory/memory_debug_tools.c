#include "memory_debug_tools.h"

#ifndef GR_DIST

#include "allocator_frontends.h"
#include "core/asserts.h"
#include "containers/hashmap_u64.h"

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
    "MEMORY DEBUG       ",
};


typedef struct MemoryDebugState
{
    HashmapU64* allocationsMap;
} MemoryDebugState;


static Allocator memoryDebugAllocator;
static MemoryDebugState* state = nullptr;


void _StartMemoryDebugSubsys()
{
    const u64 memoryDebugArenaSize = MiB * 10;
    u64 allocatorStateSize;

    if (!CreateGlobalAllocator(memoryDebugArenaSize, &memoryDebugAllocator, &allocatorStateSize))
		GRASSERT_MSG(false, "Creating memory debug allocator failed");

    state = Alloc(&memoryDebugAllocator, sizeof(*state), )
}

void _ShutdownMemoryDebugSubsys()
{
    DestroyGlobalAllocator(memoryDebugAllocator);
}

void _PrintMemoryStats()
{

}


void* DebugAlignedAlloc(Allocator* allocator, u64 size, u32 alignment)
{
    // TODO: check if the allocation is within the debug allocator, and don't do debug stuff
    return allocator->BackendAlloc(allocator, size, alignment);
}

void* DebugRealloc(Allocator* allocator, void* block, u64 newSize)
{
    // TODO: check if the allocation is within the debug allocator, and don't do debug stuff
    return allocator->BackendRealloc(allocator, block, newSize);
}

void DebugFree(Allocator* allocator, void* block)
{
    // TODO: check if the allocation is within the debug allocator, and don't do debug stuff
    allocator->BackendFree(allocator, block);
}


#endif

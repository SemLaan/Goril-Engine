#include "memory_debug_tools.h"

#ifndef GR_DIST

#include "allocator_frontends.h"
#include "core/asserts.h"
#include "core/logger.h"
#include "containers/hashmap_u64.h"
#include "containers/darray.h"

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


typedef struct AllocInfo
{
    const char* file;
    u32 line;
    MemTag tag;
    u32 allocSize;
} AllocInfo;

typedef struct MemoryDebugState
{
    u64 arenaStart;
    u64 arenaEnd;
    u64 arenaSize;
    HashmapU64* allocationsMap;
    Allocator allocInfoPool;
    u64 totalUserAllocated;
    u64 totalUserAllocationCount;
    u32 perTagAllocationCount[MAX_MEMORY_TAGS];
    u32 perTagAllocated[MAX_MEMORY_TAGS];
} MemoryDebugState;


static Allocator memoryDebugAllocator;
static MemoryDebugState* state = nullptr;


// ========================================= startup and shutdown =============================================
void _StartMemoryDebugSubsys()
{
    const u64 memoryDebugArenaSize = MiB * 10;

    u64 memoryDebugArenaStart = 0;

    if (!CreateGlobalAllocator(memoryDebugArenaSize, &memoryDebugAllocator, nullptr, &memoryDebugArenaStart))
		GRASSERT_MSG(false, "Creating memory debug allocator failed");

    state = Alloc(&memoryDebugAllocator, sizeof(*state), MEM_TAG_MEMORY_DEBUG);
    ZeroMem(state, sizeof(*state));
    state->allocationsMap = MapU64Create(&memoryDebugAllocator, MEM_TAG_MEMORY_DEBUG, 2000, 100, Hash6432Shift);
    CreatePoolAllocator(&memoryDebugAllocator, sizeof(AllocInfo), 2100, &state->allocInfoPool);
    state->totalUserAllocated = 0;
    state->totalUserAllocationCount = 0;
    state->arenaStart = memoryDebugArenaStart;
    state->arenaSize = memoryDebugArenaSize;
    state->arenaEnd = memoryDebugArenaStart + memoryDebugArenaSize;
}

void _ShutdownMemoryDebugSubsys()
{
    // Let the OS clean all of this stuff up
}

// ============================================= Memory printing utils =============================================
static const char* GetMemoryScaleString(u64 bytes, u64* out_scale)
{
	if (bytes < KiB)
	{
		*out_scale = 1;
		return "B";
	}
	else if (bytes < MiB)
	{
		*out_scale = KiB;
		return "KiB";
	}
	else if (bytes < GiB)
	{
		*out_scale = MiB;
		return "MiB";
	}
	else
	{
		*out_scale = GiB;
		return "GiB";
	}
}

void _PrintMemoryStats()
{
    GRINFO("Printing memory stats:");

	const char* scaleString;
	u64 scale;
    scaleString = GetMemoryScaleString(state->totalUserAllocated, &scale);
    GRINFO("Total user allocation count: %llu", state->totalUserAllocationCount);
    GRINFO("Total user allocated: %.2f%s", (f32)state->totalUserAllocated / (f32)scale, scaleString);

    GRINFO("Allocations by tag:");
	for (u32 i = 0; i < MAX_MEMORY_TAGS; ++i)
	{
		GRINFO("	%s: %u", memTagToText[i], state->perTagAllocationCount[i]);
	}

    AllocInfo** allocInfoDarray = (AllocInfo**)MapU64GetValueDarray(state->allocationsMap, &memoryDebugAllocator);

    GRINFO("All active allocations:");
    for (u32 i = 0; i < DarrayGetSize(allocInfoDarray); ++i)
    {
        AllocInfo* item = allocInfoDarray[i];
        GRINFO("Size: %u, File: %s:%u", item->allocSize, item->file, item->line);
    }

    DarrayDestroy(allocInfoDarray);
}

// ================================= Provides allocaters with unique id's ====================================
static u32 nextAllocatorId = 0;

// This never returns zero, it just starts counting from one
u32 _GetUniqueAllocatorId()
{
    nextAllocatorId++;
    return nextAllocatorId;
}


// ============================================= Debug alloc, realloc and free hook-ins =====================================
void* DebugAlignedAlloc(Allocator* allocator, u64 size, u32 alignment, MemTag memtag, const char* file, u32 line)
{
    GRASSERT_MSG(allocator->id > 0 && allocator->id <= nextAllocatorId, "invalid allocator id");

    Allocator* rootAllocator = allocator;
    while (rootAllocator->parentAllocator)
    {
        rootAllocator = rootAllocator->parentAllocator;
    }

    // If debug allocation
    if (rootAllocator == &memoryDebugAllocator)
    {
        return allocator->BackendAlloc(allocator, size, alignment);
    }
    else // if normal allocation
    {
        state->totalUserAllocated += size;
        state->totalUserAllocationCount++;
        state->perTagAllocated[memtag] += size;
        state->perTagAllocationCount[memtag]++;

        void* allocation = allocator->BackendAlloc(allocator, size, alignment);

        AllocInfo* allocInfo = Alloc(&state->allocInfoPool, sizeof(AllocInfo), MEM_TAG_MEMORY_DEBUG);
        allocInfo->allocSize = size;
        allocInfo->tag = memtag;
        allocInfo->file = file;
        allocInfo->line = line;
        MapU64Insert(state->allocationsMap, (u64)allocation, allocInfo);
        return allocation;
    }
}

void* DebugRealloc(Allocator* allocator, void* block, u64 newSize, const char* file, u32 line)
{
    u64 blockAddress = (u64)block;

    // If debug allocation
    if (blockAddress >= state->arenaStart && blockAddress < state->arenaEnd)
    {
        return allocator->BackendRealloc(allocator, block, newSize);
    } 
    else // if normal allocation
    {
        AllocInfo* oldAllocInfo = MapU64Delete(state->allocationsMap, (u64)block);
        if (oldAllocInfo == nullptr)
        {
            GRFATAL("Tried to realloc memory block that doesn't exists!, File: %s:%u", file, line);
            GRFATAL("Address that was attempted to be reallocated: 0x%08x", (u64)block);
            GRASSERT(false);
        }
        state->totalUserAllocated -= (oldAllocInfo->allocSize - newSize);
        state->perTagAllocated[oldAllocInfo->tag] -= (oldAllocInfo->allocSize - newSize);

        void* reallocation = allocator->BackendRealloc(allocator, block, newSize);

        AllocInfo* newAllocInfo = Alloc(&state->allocInfoPool, sizeof(AllocInfo), MEM_TAG_MEMORY_DEBUG);
        newAllocInfo->allocSize = newSize;
        newAllocInfo->tag = oldAllocInfo->tag;
        newAllocInfo->file = oldAllocInfo->file;
        newAllocInfo->line = oldAllocInfo->line;
        MapU64Insert(state->allocationsMap, (u64)reallocation, newAllocInfo);
        
        Free(&state->allocInfoPool, oldAllocInfo);

        return reallocation;
    }
}

void DebugFree(Allocator* allocator, void* block, const char* file, u32 line)
{
    u64 blockAddress = (u64)block;
    // If debug allocation
    if (blockAddress >= state->arenaStart && blockAddress < state->arenaEnd)
    {
        allocator->BackendFree(allocator, block);
    }
    else // if normal allocation
    {
        AllocInfo* allocInfo = MapU64Delete(state->allocationsMap, (u64)block);
        if (allocInfo == nullptr)
        {
            GRFATAL("Tried to free memory block that doesn't exists!, File: %s:%u", file, line);
            GRFATAL("Address that was attempted to be freed: 0x%08x", (u64)block);
            GRASSERT(false);
        }
        state->totalUserAllocationCount--;
        state->totalUserAllocated -= allocInfo->allocSize;
        state->perTagAllocated[allocInfo->tag] -= allocInfo->allocSize;
        state->perTagAllocationCount[allocInfo->tag]--;
        allocator->BackendFree(allocator, block);
        Free(&state->allocInfoPool, allocInfo);
    }
}


#endif

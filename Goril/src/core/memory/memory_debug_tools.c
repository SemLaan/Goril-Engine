#include "memory_debug_tools.h"

#ifndef GR_DIST

#include "allocator_frontends.h"
#include "containers/darray.h"
#include "containers/hashmap_u64.h"
#include "core/asserts.h"
#include "core/logger.h"
#include <stdlib.h>

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

static const char* allocatorTypeToString[ALLOCATOR_TYPE_MAX_VALUE] = {
    "global",
    "freelist",
    "bump",
    "pool",
};

typedef struct AllocInfo
{
    u32 allocatorId;
    const char* file;
    u32 line;
    MemTag tag;
    u32 allocSize;
    u32 alignment;
} AllocInfo;

typedef struct RegisteredAllocatorInfo
{
    const char* name;
    Allocator* allocator;
    u64 arenaStart;
    u64 arenaEnd;
    u32 stateSize;
    u32 allocatorId;
    u32 parentAllocatorId;
    AllocatorType type;
} RegisteredAllocatorInfo;

typedef struct MemoryDebugState
{
    u32 markedAllocatorId;
    u64 arenaStart;
    u64 arenaEnd;
    u64 arenaSize;
    RegisteredAllocatorInfo* registeredAllocatorDarray;
    HashmapU64* allocationsMap;
    Allocator* allocInfoPool;
    u64 totalUserAllocated;
    u64 totalUserAllocationCount;
    u32 perTagAllocationCount[MAX_MEMORY_TAGS];
    u32 perTagAllocated[MAX_MEMORY_TAGS];
} MemoryDebugState;

static bool memoryDebuggingAllocatorsCreated = false;
static Allocator* memoryDebugAllocator;
static MemoryDebugState* state = nullptr;

// ========================================= startup and shutdown =============================================
void _StartMemoryDebugSubsys()
{
    const u64 memoryDebugArenaSize = MiB * 10;

    u64 memoryDebugArenaStart = 0;

    if (!CreateGlobalAllocator("Debug allocator", memoryDebugArenaSize, &memoryDebugAllocator, nullptr, &memoryDebugArenaStart))
        GRASSERT_MSG(false, "Creating memory debug allocator failed");

    state = Alloc(memoryDebugAllocator, sizeof(*state), MEM_TAG_MEMORY_DEBUG);
    ZeroMem(state, sizeof(*state));
    state->allocationsMap = MapU64Create(memoryDebugAllocator, MEM_TAG_MEMORY_DEBUG, 2000, 100, Hash6432Shift);
    CreatePoolAllocator("Alloc info pool", memoryDebugAllocator, sizeof(AllocInfo), 2100, &state->allocInfoPool);
    state->totalUserAllocated = 0;
    state->totalUserAllocationCount = 0;
    state->arenaStart = memoryDebugArenaStart;
    state->arenaSize = memoryDebugArenaSize;
    state->arenaEnd = memoryDebugArenaStart + memoryDebugArenaSize;
    state->registeredAllocatorDarray = DarrayCreate(sizeof(*state->registeredAllocatorDarray), 10, memoryDebugAllocator, MEM_TAG_MEMORY_DEBUG);
    state->markedAllocatorId = UINT32_MAX;

    memoryDebuggingAllocatorsCreated = true;
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

void PrintAllocatorStatsRecursively(RegisteredAllocatorInfo* root, u32 registeredAllocatorCount, u32 depth)
{
    char* tabs = Alloc(memoryDebugAllocator, depth + 1 /*null terminator*/, MEM_TAG_MEMORY_DEBUG);
    SetMem(tabs, '\t', depth);
    tabs[depth] = 0;

    GRINFO("%s%s (id)%u, (type)%s", tabs, root->name, root->allocatorId, allocatorTypeToString[root->type]);

    u64 arenaSize = root->arenaEnd - root->arenaStart;

    const char* scaleString;
    u64 scale;
    scaleString = GetMemoryScaleString(arenaSize, &scale);
    f32 arenaSizeScaled = (f32)arenaSize / (f32)scale;
    f32 usedAmount;

    switch (root->type)
    {
    case ALLOCATOR_TYPE_GLOBAL:
    case ALLOCATOR_TYPE_FREELIST:
        usedAmount = (f32)GetFreelistAllocatorArenaUsage(root->allocator);
        GRINFO("%s%.2f/%.2f%s\t%.2f%%%% used", tabs, usedAmount / (f32)scale, arenaSizeScaled, scaleString, usedAmount / (f32)arenaSize);
        break;
    case ALLOCATOR_TYPE_BUMP:
        usedAmount = (f32)GetBumpAllocatorArenaUsage(root->allocator);
        GRINFO("%s%.2f/%.2f%s\t%.2f%%%% used", tabs, usedAmount / (f32)scale, arenaSizeScaled, scaleString, usedAmount / (f32)arenaSize);
        break;
    case ALLOCATOR_TYPE_POOL:
        usedAmount = (f32)GetPoolAllocatorArenaUsage(root->allocator);
        GRINFO("%s%.2f/%.2f%s\t%.2f%%%% used", tabs, usedAmount / (f32)scale, arenaSizeScaled, scaleString, usedAmount / (f32)arenaSize);
        break;
    default:
        GRERROR("Unknown allocator type");
        break;
    }

    Free(memoryDebugAllocator, tabs);

    for (u32 i = 0; i < registeredAllocatorCount; ++i)
    {
        if (root->allocatorId == state->registeredAllocatorDarray[i].parentAllocatorId)
            PrintAllocatorStatsRecursively(state->registeredAllocatorDarray + i, registeredAllocatorCount, depth + 1);
    }
}

void _PrintMemoryStats()
{
    GRINFO("=======================================================================================================");
    GRINFO("Printing memory stats:");

    // Printing allocators
    u32 registeredAllocatorCount = DarrayGetSize(state->registeredAllocatorDarray);
    GRINFO("Printing %u live allocators:", registeredAllocatorCount);

    RegisteredAllocatorInfo* globalAllocator = state->registeredAllocatorDarray;

    PrintAllocatorStatsRecursively(globalAllocator, registeredAllocatorCount, 1 /*start depth 1 to indent all allocators at least one tab*/);

    // Printing total allocation stats
    const char* scaleString;
    u64 scale;
    scaleString = GetMemoryScaleString(state->totalUserAllocated, &scale);
    GRINFO("Total user allocation count: %llu", state->totalUserAllocationCount);
    GRINFO("Total user allocated: %.2f%s", (f32)state->totalUserAllocated / (f32)scale, scaleString);

    // Printing allocation stats per tag
    GRINFO("Allocations by tag:");
    for (u32 i = 0; i < MAX_MEMORY_TAGS; ++i)
    {
        GRINFO("\t%s: %u", memTagToText[i], state->perTagAllocationCount[i]);
    }

    // Printing all active allocations
    // TODO: add a bool parameter to the function to specify whether to print this or not, because it's a lot
    AllocInfo** allocInfoDarray = (AllocInfo**)MapU64GetValueDarray(state->allocationsMap, memoryDebugAllocator);

    RegisteredAllocatorInfo* allocatedWithAllocator;

    GRINFO("All active allocations:");
    for (u32 i = 0; i < DarrayGetSize(allocInfoDarray); ++i)
    {
        AllocInfo* item = allocInfoDarray[i];

        allocatedWithAllocator = nullptr;

        for (u32 i = 0; i < registeredAllocatorCount; ++i)
        {
            if (state->registeredAllocatorDarray[i].allocatorId == item->allocatorId)
            {
                allocatedWithAllocator = state->registeredAllocatorDarray + i;
                break;
            }
        }

        if (allocatedWithAllocator == nullptr)
        {
            GRFATAL("Live allocation with outdated allocator: %s:%u", item->file, item->line);
            GRASSERT_MSG(false, "Live allocation with outdated allocator");
        }
        else
            GRINFO("\tAllocated by: (name)%s (id)%u (type)%s, Size: %u, File: %s:%u", allocatedWithAllocator->name, allocatedWithAllocator->allocatorId, allocatorTypeToString[allocatedWithAllocator->type], item->allocSize, item->file, item->line);
    }

    DarrayDestroy(allocInfoDarray);
    GRINFO("=======================================================================================================");
}

void _MarkAllocator(Allocator* allocator)
{
    state->markedAllocatorId = allocator->id;
}

// ================================= Registering and unregistering allocators ====================================
static u32 nextAllocatorId = 0;

// Used by allocators to identify themselves to the memory debug tools
// This never returns zero, it just starts counting from one
static u32 _GetUniqueAllocatorId()
{
    nextAllocatorId++;
    return nextAllocatorId;
}

void _RegisterAllocator(u64 arenaStart, u64 arenaEnd, u32 stateSize, u32* out_allocatorId, AllocatorType type, Allocator* parentAllocator, const char* name, Allocator* allocator)
{
    // Making sure debug allocators don't get registered, we also don't have to worry about them getting unregistered as
    // they will be cleaned up by the OS, and thus don't call unregister
    // They get an id of zero which only debug allocators can get
    if (!memoryDebuggingAllocatorsCreated)
    {
        *out_allocatorId = 0;
        return;
    }

    *out_allocatorId = _GetUniqueAllocatorId();

    RegisteredAllocatorInfo allocatorInfo = {};
    allocatorInfo.name = name;
    allocatorInfo.allocator = allocator;
    allocatorInfo.allocatorId = *out_allocatorId;
    allocatorInfo.arenaStart = arenaStart;
    allocatorInfo.arenaEnd = arenaEnd;
    allocatorInfo.stateSize = stateSize;
    allocatorInfo.type = type;
    if (parentAllocator != nullptr)
        allocatorInfo.parentAllocatorId = parentAllocator->id;
    else
        allocatorInfo.parentAllocatorId = 0;

    state->registeredAllocatorDarray = DarrayPushback(state->registeredAllocatorDarray, &allocatorInfo);
}

void _UnregisterAllocator(u32 allocatorId, AllocatorType allocatorType)
{
    u32 registeredAllocatorCount = DarrayGetSize(state->registeredAllocatorDarray);

    for (u32 i = 0; i < registeredAllocatorCount; ++i)
    {
        if (state->registeredAllocatorDarray[i].allocatorId == allocatorId)
        {
            u32 freedCount = _DebugFlushAllocator(state->registeredAllocatorDarray[i].allocator);
            if (freedCount > 0)
                GRWARN("Destroyed allocator with %u active allocation(s)", freedCount);
            DarrayPopAt(state->registeredAllocatorDarray, i);
            return;
        }
    }

    GRFATAL("Allocator with id: %u not found", allocatorId);
    GRFATAL("Allocator type: %s", allocatorTypeToString[allocatorType]);
    GRASSERT_MSG(false, "Tried to destroy allocator that wasn't found");
}

// =========================================== Flushing an allocator =======================================================
u32 _DebugFlushAllocator(Allocator* allocator)
{
    MapEntryU64** mapEntriesDarray = MapU64GetMapEntryDarray(state->allocationsMap, memoryDebugAllocator);

    u32 freedAllocations = 0;

    for (u32 i = 0; i < DarrayGetSize(mapEntriesDarray); ++i)
    {
        MapEntryU64* mapEntry = mapEntriesDarray[i];
        AllocInfo* allocInfo = mapEntry->value;

        if (allocator->id == allocInfo->allocatorId)
        {
            state->totalUserAllocationCount--;
            state->totalUserAllocated -= allocInfo->allocSize;
            state->perTagAllocated[allocInfo->tag] -= allocInfo->allocSize;
            state->perTagAllocationCount[allocInfo->tag]--;
            Free(state->allocInfoPool, allocInfo);
            MapU64Delete(state->allocationsMap, mapEntry->key);
            freedAllocations++;
        }
    }

    DarrayDestroy(mapEntriesDarray);

    return freedAllocations;
}

// ============================================= Debug alloc, realloc and free hook-ins =====================================
void* DebugAlignedAlloc(Allocator* allocator, u64 size, u32 alignment, MemTag memtag, const char* file, u32 line)
{
    // If debug allocation
    if (allocator->id == 0)
    {
        return allocator->BackendAlloc(allocator, size, alignment);
    }
    else // if normal allocation
    {
        state->totalUserAllocated += size;
        state->totalUserAllocationCount++;
        state->perTagAllocated[memtag] += size;
        state->perTagAllocationCount[memtag]++;

        void* allocation;

        if (allocator->id == state->markedAllocatorId)
            allocation = _aligned_malloc(size, alignment);
        else
            allocation = allocator->BackendAlloc(allocator, size, alignment);

        AllocInfo* allocInfo = Alloc(state->allocInfoPool, sizeof(AllocInfo), MEM_TAG_MEMORY_DEBUG);
        allocInfo->allocatorId = allocator->id;
        allocInfo->alignment = alignment;
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
    // If debug allocation
    if (allocator->id == 0)
    {
        return allocator->BackendRealloc(allocator, block, newSize);
    }
    else // if normal allocation
    {
        AllocInfo* oldAllocInfo = MapU64Delete(state->allocationsMap, (u64)block);

        // Checking if this allocation exists
        if (oldAllocInfo == nullptr)
        {
            GRFATAL("Tried to realloc memory block that doesn't exists!, File: %s:%u", file, line);
            GRFATAL("Address that was attempted to be reallocated: 0x%08x", (u64)block);
            GRASSERT(false);
        }

        // Checking if the realloc is using the wrong allocator
        if (oldAllocInfo->allocatorId != allocator->id)
        {
            GRFATAL("Tried to realloc allocation with wrong allocator!");
            GRFATAL("Allocation: %s:%u", oldAllocInfo->file, oldAllocInfo->line);
            GRFATAL("Reallocation: %s:%u", file, line);
            u32 registeredAllocatorCount = DarrayGetSize(state->registeredAllocatorDarray);
            const char* allocatorName;
            for (u32 i = 0; i < registeredAllocatorCount; ++i)
            {
                if (state->registeredAllocatorDarray[i].allocatorId == allocator->id)
                {
                    allocatorName = state->registeredAllocatorDarray[i].name;
                }
            }
            GRFATAL("Wrong allocator: %s", allocatorName);
            for (u32 i = 0; i < registeredAllocatorCount; ++i)
            {
                if (state->registeredAllocatorDarray[i].allocatorId == oldAllocInfo->allocatorId)
                {
                    allocatorName = state->registeredAllocatorDarray[i].name;
                }
            }
            GRFATAL("Correct allocator: %s", allocatorName);
            GRASSERT(false);
        }

        state->totalUserAllocated -= (oldAllocInfo->allocSize - newSize);
        state->perTagAllocated[oldAllocInfo->tag] -= (oldAllocInfo->allocSize - newSize);

        void* reallocation;

        if (allocator->id == state->markedAllocatorId)
            reallocation = _aligned_realloc(block, newSize, oldAllocInfo->alignment);
        else
            reallocation = allocator->BackendRealloc(allocator, block, newSize);

        AllocInfo* newAllocInfo = Alloc(state->allocInfoPool, sizeof(AllocInfo), MEM_TAG_MEMORY_DEBUG);
        newAllocInfo->allocatorId = oldAllocInfo->allocatorId;
        newAllocInfo->allocSize = newSize;
        newAllocInfo->tag = oldAllocInfo->tag;
        newAllocInfo->file = oldAllocInfo->file;
        newAllocInfo->line = oldAllocInfo->line;
        MapU64Insert(state->allocationsMap, (u64)reallocation, newAllocInfo);

        Free(state->allocInfoPool, oldAllocInfo);

        return reallocation;
    }
}

void DebugFree(Allocator* allocator, void* block, const char* file, u32 line)
{
    // If debug allocation
    if (allocator->id == 0)
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

        // Checking if the free is using the wrong allocator
        if (allocInfo->allocatorId != allocator->id)
        {
            GRFATAL("Tried to free allocation with wrong allocator!");
            GRFATAL("Allocation: %s:%u", allocInfo->file, allocInfo->line);
            GRFATAL("Free: %s:%u", file, line);
            u32 registeredAllocatorCount = DarrayGetSize(state->registeredAllocatorDarray);
            const char* allocatorName;
            for (u32 i = 0; i < registeredAllocatorCount; ++i)
            {
                if (state->registeredAllocatorDarray[i].allocatorId == allocator->id)
                {
                    allocatorName = state->registeredAllocatorDarray[i].name;
                }
            }
            GRFATAL("Wrong allocator: %s", allocatorName);
            for (u32 i = 0; i < registeredAllocatorCount; ++i)
            {
                if (state->registeredAllocatorDarray[i].allocatorId == allocInfo->allocatorId)
                {
                    allocatorName = state->registeredAllocatorDarray[i].name;
                }
            }
            GRFATAL("Correct allocator: %s", allocatorName);
            GRASSERT(false);
        }

        state->totalUserAllocationCount--;
        state->totalUserAllocated -= allocInfo->allocSize;
        state->perTagAllocated[allocInfo->tag] -= allocInfo->allocSize;
        state->perTagAllocationCount[allocInfo->tag]--;
        Free(state->allocInfoPool, allocInfo);

        if (allocator->id == state->markedAllocatorId)
            _aligned_free(block);
        else
            allocator->BackendFree(allocator, block);
    }
}

#endif

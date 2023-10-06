#include "hashmap_str.h"

#include <string.h>
#include "core/asserts.h"

HashmapStr* MapStrCreate(Allocator* allocator, mem_tag memtag, u32 backingArrayElementCount, u32 maxCollisions, u32 maxKeyLength, HashFunctionStr hashFunction)
{
    HashmapStr* hashmap = Alloc(allocator, sizeof(*hashmap) + backingArrayElementCount * sizeof(MapEntryStr), memtag);
    hashmap->backingArray = (MapEntryStr*)(hashmap + 1);
    hashmap->backingArrayElementCount = backingArrayElementCount;
    hashmap->hashFunction = hashFunction;
    hashmap->linkedEntryPool = CreatePoolAllocator(sizeof(MapEntryStr), maxCollisions);
    hashmap->keyPool = CreatePoolAllocator(maxKeyLength, maxCollisions + backingArrayElementCount);
    hashmap->allocator = allocator;
    hashmap->maxKeyLength = maxKeyLength;

    memset(hashmap->backingArray, 0, sizeof(MapEntryStr) * backingArrayElementCount);

    return hashmap;
}

void MapStrDestroy(HashmapStr* hashmap)
{
    DestroyPoolAllocator(hashmap->linkedEntryPool);
    DestroyPoolAllocator(hashmap->keyPool);

    Free(hashmap->allocator, hashmap);
}

void MapStrInsert(HashmapStr* hashmap, const char* key, u32 keyLength, void* value)
{
    u32 hash = hashmap->hashFunction(key, keyLength) % hashmap->backingArrayElementCount;

    // TODO: when in debug build check if the entry is already in the map

    MapEntryStr* currentEntry = &hashmap->backingArray[hash];

    while (nullptr != currentEntry->key)
    {
        if (currentEntry->next != nullptr)
        {
            currentEntry = currentEntry->next;
        }
        else
        {
            currentEntry->next = Alloc(&hashmap->linkedEntryPool, sizeof(MapEntryStr), MEM_TAG_HASHMAP);
            memset(currentEntry->next, 0, sizeof(MapEntryStr));
            currentEntry = currentEntry->next;
        }
    }

    currentEntry->key = Alloc(&hashmap->keyPool, hashmap->maxKeyLength, MEM_TAG_HASHMAP);
    GRASSERT_DEBUG(keyLength < hashmap->maxKeyLength);
    MemCopy(currentEntry->key, key, keyLength);
    currentEntry->keyLength = keyLength;
    currentEntry->value = value;
}


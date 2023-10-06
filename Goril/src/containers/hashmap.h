#pragma once

#include "defines.h"
#include "core/gr_memory.h"

typedef u32 (*HashFunction)(const char* key, u32 keyLength);


typedef struct MapEntry
{
    char* key;
    void* value;
    struct MapEntry* next;
    u32 keyLength;
} MapEntry;

typedef struct Hashmap
{
    HashFunction hashFunction;
    Allocator linkedEntryPool;
    Allocator keyPool;
    Allocator* allocator;
    MapEntry* backingArray;
    u32 backingArrayElementCount;
    u32 maxKeyLength;
} Hashmap;

// Creates a map, objects are to be kept track of outside of the hashmap
// note that the hashmap stores pointers to objects, so those pointers can not be invalidated
// or the hashmap has outdated pointers
// backingArrayElementCount: amount of elements in the backing array
Hashmap* MapCreate(Allocator* allocator, mem_tag memtag, u32 backingArrayElementCount, u32 maxCollisions, u32 maxKeyLength, HashFunction hashFunction);

// Destroys everything about the map, except the objects
void MapDestroy(Hashmap* hashmap);

// Inserts item into map, asserts if the key is already in the map
void MapInsert(Hashmap* hashmap, const char* key, u32 keyLength, void* value);

// Returns a void pointer to the found object or nullptr if the object wasn't found
void* MapLookup(Hashmap* hashmap, const char* key, u32 keyLength);

// Returns the deleted element
void* MapDelete(Hashmap* hashmap, const char* key, u32 keyLength);
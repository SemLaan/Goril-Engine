#pragma once

#include "defines.h"
#include "core/meminc.h"


typedef u32 (*HashFunctionU64)(u64 key);


typedef struct MapEntryU64
{
    u64 key;
    void* value;
    struct MapEntryU64* next;
} MapEntryU64;

typedef struct HashmapU64
{
    HashFunctionU64 hashFunction;
    Allocator linkedEntryPool;
    Allocator* allocator;
    MapEntryU64* backingArray;
    u32 backingArrayElementCount;
} HashmapU64;

// Creates a map, objects are to be kept track of outside of the hashmap
// note that the hashmap stores pointers to objects, so those pointers can not be invalidated
// or the hashmap has outdated pointers
// backingArrayElementCount: amount of elements in the backing array
HashmapU64* MapU64Create(Allocator* allocator, mem_tag memtag, u32 backingArrayElementCount, u32 maxCollisions, HashFunctionU64 hashFunction);

// Destroys everything about the map, except the objects
void MapU64Destroy(HashmapU64* hashmap);

// Inserts item into map, asserts if the key is already in the map
void MapU64Insert(HashmapU64* hashmap, u64 key, void* value);

// Returns a void pointer to the found object or nullptr if the object wasn't found
void* MapU64Lookup(HashmapU64* hashmap, u64 key);

// Returns the deleted element
void* MapU64Delete(HashmapU64* hashmap, u64 key);
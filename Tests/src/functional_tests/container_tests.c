#include "test_register_functions.h"

#include "test_defines.h"
#include "../test_manager.h"
#include <containers/darray.h>
#include <containers/hashmap_u64.h>


typedef struct Beef
{
	u32 amount;
	f32 quality;
} Beef;


static bool darray_pushback_test()
{
	Beef* darray = (Beef*)DarrayCreate(sizeof(Beef), 1, GetGlobalAllocator(), MEM_TAG_TEST);

	u32 amountValue = 3;
	f32 qualityValue = 2.3f;

	Beef test = {amountValue, qualityValue};
	darray = (Beef*)DarrayPushback(darray, &test);
	darray = (Beef*)DarrayPushback(darray, &test);
	darray = (Beef*)DarrayPushback(darray, &test);
	darray = (Beef*)DarrayPushback(darray, &test);

	expect_float_to_be(qualityValue, darray[0].quality);
	expect_should_be(amountValue, darray[0].amount);
	expect_float_to_be(qualityValue, darray[3].quality);
	expect_should_be(amountValue, darray[3].amount);

	DarrayDestroy(darray);

	return true;
}

static bool darray_pop_test()
{
	Beef* darray = (Beef*)DarrayCreate(sizeof(Beef), 1, GetGlobalAllocator(), MEM_TAG_TEST);

	u32 amountValue = 3;
	f32 qualityValue = 2.3f;

	Beef test1 = {0, 0};
	Beef test2 = {amountValue, qualityValue};

	darray = (Beef*)DarrayPushback(darray, &test1);
	darray = (Beef*)DarrayPushback(darray, &test1);
	darray = (Beef*)DarrayPushback(darray, &test1); // 2, this one gets popped
	darray = (Beef*)DarrayPushback(darray, &test2);

	DarrayPopAt(darray, 2);

	expect_float_to_be(qualityValue, darray[2].quality);
	expect_should_be(amountValue, darray[2].amount);

	DarrayPop(darray);

	expect_float_to_be(0, darray[1].quality);
	expect_should_be(0, darray[1].amount);

	DarrayDestroy(darray);

	return true;
}

static bool darray_contains_test()
{
	int* darray = (int*)DarrayCreate(sizeof(int), 1, GetGlobalAllocator(), MEM_TAG_TEST);

	int i = 1;
	darray = (int*)DarrayPushback(darray, &i);
	i = 3;
	darray = (int*)DarrayPushback(darray, &i);
	i = 4;
	darray = (int*)DarrayPushback(darray, &i);
	i = 5;
	darray = (int*)DarrayPushback(darray, &i);

	i = 4;
	expect_to_be_true(DarrayContains(darray, &i));
	i = 2;
	expect_to_be_false(DarrayContains(darray, &i));

	DarrayDestroy(darray);

	return true;
}

static u32 TestHashFunc(u64 key)
{
	// Making sure 10 is a collision with 5
	if (key == 10)
		return 5;
	else
		return (u32)key;
}

#include "stdlib.h"
static bool hashmapU64_test()
{
	#define testElementCount 20

	HashmapU64* hashmap = MapU64Create(GetGlobalAllocator(), MEM_TAG_TEST, 100, 20, TestHashFunc);

	u32* testValues = Alloc(GetGlobalAllocator(), sizeof(*testValues) * testElementCount, MEM_TAG_TEST);

	for (u32 i = 0; i < testElementCount; ++i)
	{
		testValues[i] = i;
	}

	for (u64 i = 0; i < testElementCount; ++i)
	{
		MapU64Insert(hashmap, i, &testValues[i]);
	}

	for (u64 i = 0; i < testElementCount; ++i)
	{
		u32* value = MapU64Lookup(hashmap, i);
		expect_should_be(i, *value);
	}

	MapU64Delete(hashmap, (u64)4);
	MapU64Delete(hashmap, (u64)10);

	for (u64 i = 0; i < testElementCount; ++i)
	{
		if (i == 4 || i == 10)
			continue;
		u32* value = MapU64Lookup(hashmap, i);
		expect_should_be(i, *value);
	}

	Free(GetGlobalAllocator(), testValues);

	MapU64Destroy(hashmap);

	return true;
}


void register_container_tests()
{
	register_test(darray_pushback_test, "Darray: pushback");
	register_test(darray_pop_test, "Darray: pop");
	register_test(darray_contains_test, "Darray: contains");
	register_test(hashmapU64_test, "Hashmap u64");
}

#include "container_tests.h"

#include "test_defines.h"
#include "../test_manager.h"
#include <containers/darray.h>


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


void register_container_tests()
{
	register_test(darray_pushback_test, "Darray: pushback");
	register_test(darray_pop_test, "Darray: pop");
	register_test(darray_contains_test, "Darray: contains");
}

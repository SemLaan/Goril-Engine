#include "darray_test.h"

#include "test_defines.h"
#include "../test_manager.h"
#include <containers/darray.h>

using namespace GR;

struct Beef
{
	u32 amount;
	f32 quality;
};


static b8 darray_pushback_test()
{
	Darray<Beef> darray = Darray<Beef>();
	darray.Initialize();

	u32 amountValue = 3;
	f32 qualityValue = 2.3f;

	darray.Pushback(Beef(amountValue, qualityValue));
	darray.Pushback(Beef(amountValue, qualityValue));
	darray.Pushback(Beef(amountValue, qualityValue));
	darray.Pushback(Beef(amountValue, qualityValue));

	expect_float_to_be(qualityValue, darray[0].quality);
	expect_should_be(amountValue, darray[0].amount);
	expect_float_to_be(qualityValue, darray[3].quality);
	expect_should_be(amountValue, darray[3].amount);

	darray.Deinitialize();

	return true;
}

static b8 darray_pop_test()
{
	Darray<Beef> darray = Darray<Beef>();
	darray.Initialize();

	u32 amountValue = 3;
	f32 qualityValue = 2.3f;

	darray.Pushback(Beef(0, 0));
	darray.Pushback(Beef(0, 0));
	darray.Pushback(Beef(0, 0)); // 2, this one gets poped
	darray.Pushback(Beef(amountValue, qualityValue));

	darray.PopAt(2);

	expect_float_to_be(qualityValue, darray[2].quality);
	expect_should_be(amountValue, darray[2].amount);

	darray.Pop();

	expect_float_to_be(0, darray[1].quality);
	expect_should_be(0, darray[1].amount);

	darray.Deinitialize();

	return true;
}

static b8 darray_contains_test()
{
	Darray<int> darray = Darray<int>();
	darray.Initialize();

	darray.Pushback(1);
	darray.Pushback(3);
	darray.Pushback(4);
	darray.Pushback(5);

	expect_to_be_true(darray.Contains(4));
	expect_to_be_false(darray.Contains(2));

	darray.Deinitialize();

	return true;
}


void register_darray_tests()
{
	register_test(darray_pushback_test, "Darray: pushback");
	register_test(darray_pop_test, "Darray: pop");
	register_test(darray_contains_test, "Darray: contains");
}

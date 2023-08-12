#include "darray_test.h"

#include "test_defines.h"
#include "../test_manager.h"
#include <goril.h>
#include <containers/darray.h>

using namespace GR;

struct Beef
{
	u32 amount;
	f32 quality;
};


b8 darray_pushback_test()
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

b8 darray_pop_test()
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


void register_darray_tests()
{
	register_test(darray_pushback_test, "Darray: pushback");
	register_test(darray_pop_test, "Darray: pop");
}

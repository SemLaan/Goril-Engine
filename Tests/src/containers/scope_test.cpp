#include "scope_test.h"

#include "test_defines.h"
#include "../test_manager.h"
#include <goril.h>

using namespace GR;


class Example
{
public:
	i32 testInt;

	Example(i32 i)
		: testInt(i)
	{}
};


b8 object_creation_test()
{
	i32 testValue = 5;

	Scope<Example> temp = CreateScope<Example>(GetGlobalAllocator(), mem_tag::TEST, testValue);

	expect_should_be(temp->testInt, testValue);

	return true;
}

b8 out_of_scope_test()
{
	i32 testValue = 5;

	u64 startAllocations = GetNetAllocations();

	{
		Scope<Example> temp = CreateScope<Example>(GetGlobalAllocator(), mem_tag::TEST, testValue);

		expect_to_be_true(GetNetAllocations() > startAllocations);
	}

	expect_should_be(startAllocations, GetNetAllocations());
	
	return true;
}

b8 reset_test()
{
	i32 testValue = 5;

	u64 startAllocations = GetNetAllocations();


	Scope<Example> temp = CreateScope<Example>(GetGlobalAllocator(), mem_tag::TEST, testValue);

	expect_to_be_true(GetNetAllocations() > startAllocations);

	temp.Reset();

	expect_should_be(startAllocations, GetNetAllocations());

	return true;
}


void register_scope_tests()
{
	register_test(object_creation_test, "Scope: object creation");
	register_test(out_of_scope_test, "Scope: Out of scope object destruction");
	register_test(reset_test, "Scope: Reset object destruction");
}

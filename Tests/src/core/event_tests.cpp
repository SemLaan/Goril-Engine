#include "event_tests.h"

#include "test_defines.h"
#include "../test_manager.h"
#include <core/event.h>


static EventData g_data;
static EventData g_data1;
static EventData g_data2;

static bool TestListener(EventCode type, EventData data)
{
	g_data = data;
	//GRDEBUG("Test, %u, %u, %u, %u", data.u32[0], data.u32[1], data.u32[2], data.u32[3]);
	return false;
}

static bool TestListener1(EventCode type, EventData data)
{
	g_data1 = data;
	//GRDEBUG("Test, %u, %u, %u, %u", data.u32[0], data.u32[1], data.u32[2], data.u32[3]);
	return true;
}

static bool TestListener2(EventCode type, EventData data)
{
	g_data2 = data;
	//GRDEBUG("Test, %u, %u, %u, %u", data.u32[0], data.u32[1], data.u32[2], data.u32[3]);
	return true;
}

static bool event_test()
{
	g_data = {};
	RegisterEventListener(EVCODE_TEST, TestListener);

	EventData testData{};
	testData.u32[0] = 3;
	testData.u32[1] = 4;
	testData.u32[2] = 5;
	testData.u32[3] = 6;

	InvokeEvent(EVCODE_TEST, testData);

	expect_should_be(g_data.u64[0], testData.u64[0]);
	expect_should_be(g_data.u64[1], testData.u64[1]);

	UnregisterEventListener(EVCODE_TEST, TestListener);

	RegisterEventListener(EVCODE_TEST, TestListener1);
	RegisterEventListener(EVCODE_TEST, TestListener2);

	testData = {};
	testData.b8[0] = true;

	InvokeEvent(EVCODE_TEST, testData);

	expect_to_be_true(g_data1.b8[0]);
	expect_to_be_false(g_data2.b8[0]);

	UnregisterEventListener(EVCODE_TEST, TestListener1);
	UnregisterEventListener(EVCODE_TEST, TestListener2);

	return true;
}


void register_event_tests()
{
	register_test(event_test, "Events: test");
}
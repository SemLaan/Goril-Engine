#include "timer_tests.h"

#include "../test_manager.h"
#include <goril.h>
#include <chrono>
#include <thread>
#include "test_defines.h"

#define MILLISECONDS_TO_SECONDS(x) (x / 1000)
#define SECONDS_TO_MILLISECONDS(x) (x * 1000)

b8 timer_test()
{
	f64 ms_tolerance = 50;
	f64 seconds_tolerance = MILLISECONDS_TO_SECONDS(ms_tolerance);
	u32 sleep_ms = 1000;
	f64 sleep_seconds = MILLISECONDS_TO_SECONDS(sleep_ms);

	GR::Timer timer = GR::Timer();
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));

	f64 ms_time = timer.MilisecondsSinceStart();
	f64 seconds_time = timer.SecondsSinceStart();

	expect_to_be_true((std::abs((f64)sleep_ms - ms_time) < ms_tolerance));
	expect_to_be_true(std::abs(sleep_seconds - seconds_time) < seconds_tolerance);

	return true;
}


void register_timer_tests()
{
	register_test(timer_test);
}
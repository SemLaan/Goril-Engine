#pragma once
#include <goril.h>


typedef b8(*PFN_test)();

b8 initialize_test_manager();

void shutdown_test_manager();

void register_test(PFN_test test);

b8 run_tests();
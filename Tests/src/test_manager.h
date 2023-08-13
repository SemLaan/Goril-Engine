#pragma once
#include "defines.h"
#include <string>

typedef b8(*PFN_test)();

b8 initialize_test_manager();

void shutdown_test_manager();

void register_test(PFN_test test, std::string test_name);

b8 run_tests();
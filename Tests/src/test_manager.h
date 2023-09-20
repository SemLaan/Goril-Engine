#pragma once
#include "defines.h"
#include <string>

typedef bool(*PFN_test)();

bool initialize_test_manager();

void shutdown_test_manager();

void register_test(PFN_test test, const char* test_name);

bool run_tests();
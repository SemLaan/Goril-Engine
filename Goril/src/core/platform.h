#pragma once
#include "defines.h"
#include "logger.h"

namespace gr
{
	b8 initialize_platform();

	void shutdown_platform();

	void platform_log_message(log_level level, const char* message);
}
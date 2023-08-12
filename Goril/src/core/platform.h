#pragma once
#include "defines.h"
#include "logger.h"

namespace GR
{
	b8 InitializePlatform();

	void ShutdownPlatform();

	void PlatformLogString(log_level level, const char* message);
}
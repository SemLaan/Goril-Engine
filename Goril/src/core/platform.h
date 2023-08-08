#pragma once
#include "defines.h"
#include "logger.h"

namespace GR
{
	size_t GetPlatformRequiredMemory();

	b8 InitializePlatform();

	void ShutdownPlatform();

	void PlatformLogMessage(log_level level, const char* message);
}
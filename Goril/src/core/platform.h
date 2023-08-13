#pragma once
#include "defines.h"
#include "logger.h"

namespace GR
{
	b8 InitializePlatform(const wchar_t* windowName, b8 startMinimized);

	void ShutdownPlatform();

	void PlatformProcessMessage();

	void PlatformLogString(log_level level, const char* message);
}
#pragma once
#include "defines.h"
#include "logger.h"

namespace GR
{
	b8 InitializePlatform(const wchar_t* windowName);

	void ShutdownPlatform();

	void PlatformProcessMessage();

	void PlatformLogString(log_level level, const char* message);
}
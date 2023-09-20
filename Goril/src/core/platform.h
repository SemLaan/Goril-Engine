#pragma once
#include "defines.h"
#include "logger.h"
#include "math/math_types.h"


bool InitializePlatform(const wchar_t* windowName);

void ShutdownPlatform();

// Exported for testing, shouldn't be used by applications
GRAPI void PlatformProcessMessage();

void PlatformLogString(log_level level, const char* message);

GRAPI vec2i GetPlatformWindowSize();

void SetMousePosition(vec2i position);
void SetWindowTitle(const wchar_t* windowName);

GRAPI void ToggleFullscreen();
GRAPI void SetFullscreen(bool enabled);

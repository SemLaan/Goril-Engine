#pragma once
#include "defines.h"
#include "core/logger.h"
#include "math/math_types.h"


bool InitializePlatform(const char* windowName);

void ShutdownPlatform();

// Exported for testing, shouldn't be used by applications
GRAPI void PlatformProcessMessage();

void PlatformLogString(log_level level, const char* message);

GRAPI vec2i GetPlatformWindowSize();

void SetMousePosition(vec2i position);
void SetWindowTitle(const char* windowName);

GRAPI void ToggleFullscreen();
GRAPI void SetFullscreen(bool enabled);

// Returns time since system boot in seconds
f64 PlatformGetTime();
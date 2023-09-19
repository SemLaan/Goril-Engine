#pragma once
#include "defines.h"
#include "logger.h"
#include "glm/glm.hpp"



bool InitializePlatform(const wchar_t* windowName);

void ShutdownPlatform();

// Exported for testing, shouldn't be used by applications
GRAPI void PlatformProcessMessage();

void PlatformLogString(log_level level, const char* message);

GRAPI glm::ivec2 GetPlatformWindowSize();

void SetMousePosition(glm::ivec2 position);
void SetWindowTitle(const wchar_t* windowName);

GRAPI void ToggleFullscreen();
GRAPI void SetFullscreen(bool enabled);

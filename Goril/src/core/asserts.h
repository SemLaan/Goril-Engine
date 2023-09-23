#pragma once
#include "logger.h"

#ifdef __GNUC__
#define debugBreak() __builtin_trap()
#else
#define debugBreak() __debugbreak()
#endif

#define GRASSERT(expr) {if (expr){} else { GRFATAL("Assertion fail: %s, File: %s, Line: %i", #expr, __FILE__, __LINE__); debugBreak();}}
#define GRASSERT_MSG(expr, message) {if (expr){} else { GRFATAL("Assertion fail: %s, Message: %s, File: %s, Line: %i", #expr, message, __FILE__, __LINE__); debugBreak();}}

#ifdef GR_DEBUG
#define GRASSERT_DEBUG(expr) {if (expr){} else { GRFATAL("Assertion fail: %s, File: %s, Line: %i", #expr, __FILE__, __LINE__); debugBreak();}}
#else
#define GRASSERT_DEBUG(expr)
#endif
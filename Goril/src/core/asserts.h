#pragma once
#include "logger.h"

//#include <intrin.h>
#define debugBreak() __debugbreak()

#define GRASSERT(expr) {if (expr){} else { GRFATAL("Assertion fail: %s, File: %s, Line: %s", #expr, __FILE__, __LINE__); debugBreak();}}
#define GRASSERT_MSG(expr, message) {if (expr){} else { GRFATAL("Assertion fail: %s, Message: %s, File: %s, Line: %s", #expr, message, __FILE__, __LINE__); debugBreak();}}

#ifdef GR_DEBUG
#define GRASSERT_DEBUG(expr) {if (expr){} else { GRFATAL("Assertion fail: %s, File: %s, Line: %s", #expr, __FILE__, __LINE__); debugBreak();}}
#else
#define GRASSERT_DEBUG(expr)
#endif
#pragma once
#include "logger.h"

#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

#define GRASSERT(expr) {if (expr){} else { GRFATAL("Assertion fail: {0} {1} {2} {3} {4}", #expr, "File:", __FILE__, "Line:", __LINE__); debugBreak();}}
#define GRASSERT_MSG(expr, message) {if (expr){} else { GRFATAL("Assertion fail: {0} {1} {2} {3} {4} {5} {6}", #expr, "Message:", message, "File:", __FILE__, "Line:", __LINE__); debugBreak();}}

#ifdef GR_DEBUG
#define GRASSERT_DEBUG(expr) {if (expr){} else { GRFATAL("Assertion fail: {0} {1} {2} {3} {4}", #expr, "File:", __FILE__, "Line:", __LINE__); debugBreak();}}
#else
#define GRASSERT_DEBUG(expr)
#endif
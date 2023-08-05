#pragma once
#include "logger.h"

//#include <intrin.h>
#define debugBreak() __debugbreak()

#define GRASSERT(expr) {if (expr){} else { GRFATAL("Assertion fail: {0}, File: {1}, Line: {2}", #expr, __FILE__, __LINE__); debugBreak();}}
#define GRASSERT_MSG(expr, message) {if (expr){} else { GRFATAL("Assertion fail: {0}, Message: {1}, File: {2}, Line: {3}", #expr, message, __FILE__, __LINE__); debugBreak();}}

#ifdef GR_DEBUG
#define GRASSERT_DEBUG(expr) {if (expr){} else { GRFATAL("Assertion fail: {0}, File: {1}, Line: {2}", #expr, __FILE__, __LINE__); debugBreak();}}
#else
#define GRASSERT_DEBUG(expr)
#endif
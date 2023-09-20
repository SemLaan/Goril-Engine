#pragma once
#include "defines.h"

enum log_level
{
	LOG_LEVEL_FATAL = 0,
	LOG_LEVEL_ERROR = 1,
	LOG_LEVEL_WARN = 2,
	LOG_LEVEL_INFO = 3,
	LOG_LEVEL_DEBUG = 4,
	LOG_LEVEL_TRACE = 5,
	MAX_LOG_LEVELS
};


void WriteLogsToFile();

GRAPI void Log(log_level level, const char* message, ...);


// Always define fatal and error
#define GRFATAL(message, ...)	Log(LOG_LEVEL_FATAL, message, __VA_ARGS__)
#define GRERROR(message, ...)	Log(LOG_LEVEL_ERROR, message, __VA_ARGS__)

#ifndef GR_DIST
#define GRWARN(message, ...)	Log(LOG_LEVEL_WARN, message, __VA_ARGS__)
#define GRINFO(message, ...)	Log(LOG_LEVEL_INFO, message, __VA_ARGS__)
#else
#define GRWARN(message, ...)
#define GRINFO(message, ...)
#endif

#ifdef GR_DEBUG
#define GRDEBUG(message, ...)	Log(LOG_LEVEL_DEBUG, message, __VA_ARGS__)
#define GRTRACE(message, ...)	Log(LOG_LEVEL_TRACE, message, __VA_ARGS__)
#else
#define GRDEBUG(message, ...)
#define GRTRACE(message, ...)
#endif
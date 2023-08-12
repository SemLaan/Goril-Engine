#pragma once
#include "defines.h"
#include <string>
#include <format>

namespace GR
{
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

	b8 InitializeLogger();

	void ShutdownLogger();
	
	GRAPI void Log(log_level level, std::string message);
}

// Always define fatal and error
#define GRFATAL(message, ...)	::GR::Log(::GR::LOG_LEVEL_FATAL, std::format(message, __VA_ARGS__))
#define GRERROR(message, ...)	::GR::Log(::GR::LOG_LEVEL_ERROR, std::format(message, __VA_ARGS__))

#ifndef GR_DIST
#define GRWARN(message, ...)	::GR::Log(::GR::LOG_LEVEL_WARN, std::format(message, __VA_ARGS__))
#define GRINFO(message, ...)	::GR::Log(::GR::LOG_LEVEL_INFO, std::format(message, __VA_ARGS__))
#else
#define GRWARN(message, ...)
#define GRINFO(message, ...)
#endif

#ifdef GR_DEBUG
#define GRDEBUG(message, ...)	::GR::Log(::GR::LOG_LEVEL_DEBUG, std::format(message, __VA_ARGS__))
#define GRTRACE(message, ...)	::GR::Log(::GR::LOG_LEVEL_TRACE, std::format(message, __VA_ARGS__))
#else
#define GRDEBUG(message, ...)
#define GRTRACE(message, ...)
#endif
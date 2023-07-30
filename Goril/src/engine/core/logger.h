#pragma once
#include "gorilmem.h"
#include <spdlog/spdlog.h>

namespace Goril
{

	class Logger
	{
	public:
		void Init();
		void Shutdown();

		inline static Ref<spdlog::logger>& GetLogger() { return s_logger; }

	private:
		static Ref<spdlog::logger> s_logger;

		// Singleton code
	public:
		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;
		Logger& operator=(Logger&&) = delete;
		Logger(Logger&&) = delete;

		static Logger*& Get();

	private:
		Logger() = default;
	};
}

// Always define fatal and error
#define GRFATAL(...)	::Goril::Logger::GetLogger()->critical(__VA_ARGS__)
#define GRERROR(...)	::Goril::Logger::GetLogger()->error(__VA_ARGS__)

#ifndef GR_DIST
#define GRWARN(...)		::Goril::Logger::GetLogger()->warn(__VA_ARGS__)
#define GRINFO(...)		::Goril::Logger::GetLogger()->info(__VA_ARGS__)
#else
#define GRWARN(...)
#define GRINFO(...)
#endif

#ifdef GR_DEBUG
#define GRDEBUG(...)	::Goril::Logger::GetLogger()->debug(__VA_ARGS__)
#define GRTRACE(...)	::Goril::Logger::GetLogger()->trace(__VA_ARGS__)
#else
#define GRDEBUG(...)
#define GRTRACE(...)
#endif
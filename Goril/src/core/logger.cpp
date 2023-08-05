#include "logger.h"

#include "platform.h"

namespace gr
{

	static std::string log_levels[MAX_LOG_LEVELS] =
	{
		"[FATAL]: ",
		"[ERROR]: ",
		"[WARN]:  ",
		"[INFO]:  ",
		"[DEBUG]: ",
		"[TRACE]: ",
	};

	b8 initialize_logger()
	{
		// TODO: create file to write logs to

		// Testing logging functions
		GRFATAL("Testing log functions: {}", "succesfull");
		GRERROR("Testing log functions: {}", "succesfull");
		GRWARN("Testing log functions: {}", "succesfull");
		GRINFO("Testing log functions: {}", "succesfull");
		GRDEBUG("Testing log functions: {}", "succesfull");
		GRTRACE("Testing log functions: {}", "succesfull");

		return true;
	}

	void shutdown_logger()
	{
		// TODO: write logs to file
	}

	void log(log_level level, std::string message)
	{
		platform_log_message(level, (log_levels[level] + message + "\n").c_str());
	}
}
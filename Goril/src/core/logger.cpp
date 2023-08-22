#include "logger.h"

#include "platform.h"
#include "asserts.h"
#include "gr_memory.h"
#include <fstream>

namespace GR
{

	static std::string logLevels[MAX_LOG_LEVELS] =
	{
		"[FATAL]: ",
		"[ERROR]: ",
		"[WARN]:  ",
		"[INFO]:  ",
		"[DEBUG]: ",
		"[TRACE]: ",
	};


	static std::ofstream file("console.log");


	void WriteLogsToFile()
	{
		GRINFO("Writing logs to file...");

		file.close();
	}

	void Log(log_level level, std::string message)
	{
		std::string string = logLevels[level] + message + "\n";
		file << string.c_str();
		PlatformLogString(level, string.c_str());
	}
}
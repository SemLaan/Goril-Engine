#include "logger.h"

#include "platform.h"
#include "asserts.h"
#include "gr_memory.h"
#include <stdio.h>



static std::string logLevels[MAX_LOG_LEVELS] =
{
	"[FATAL]: ",
	"[ERROR]: ",
	"[WARN]:  ",
	"[INFO]:  ",
	"[DEBUG]: ",
	"[TRACE]: ",
};


static FILE* file = nullptr;


void WriteLogsToFile()
{
	GRINFO("Writing logs to file...");

	if (file != nullptr)
		fclose(file);
}

void Log(log_level level, std::string message)
{
	std::string string = logLevels[level] + message + "\n";

	if (file == nullptr)
		file = fopen("console.log", "w+");
	fprintf(file, string.c_str());
	PlatformLogString(level, string.c_str());
}

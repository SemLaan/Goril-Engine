#include "logger.h"

#include "platform.h"
#include "asserts.h"
#include "gr_memory.h"
#include <stdio.h>
#include <stdarg.h>


// If the log string exceeds this size it will log a warning and the full message will not get logged
#define MAX_LOG_CHARS 4000

// NOTE: this is without the null terminator on purpose
#define LOG_LEVEL_STRING_SIZE 10

#define MAX_USER_LOG_CHARS (MAX_LOG_CHARS - LOG_LEVEL_STRING_SIZE)


static const char* logLevels[MAX_LOG_LEVELS] =
{
	"\n[FATAL]: ",
	"\n[ERROR]: ",
	"\n[WARN]:  ",
	"\n[INFO]:  ",
	"\n[DEBUG]: ",
	"\n[TRACE]: ",
};


static FILE* file = nullptr;


void WriteLogsToFile()
{
	GRINFO("Writing logs to file...");

	if (file != nullptr)
		fclose(file);
}

void Log(log_level level, const char* message, ...)
{
	char final_message[MAX_LOG_CHARS];

	MemCopy(final_message, (void*)logLevels[level], LOG_LEVEL_STRING_SIZE);

	char* user_message = final_message + LOG_LEVEL_STRING_SIZE;

	va_list args;
	va_start(args, message);

	i32 result = vsnprintf(user_message, MAX_USER_LOG_CHARS, message, args);

	va_end(args);

	if (file == nullptr)
		file = fopen("console.log", "w+");
	fprintf(file, final_message);
	PlatformLogString(level, final_message);

	if (result >= MAX_USER_LOG_CHARS || result < 0)
		PlatformLogString(LOG_LEVEL_FATAL, "\nLogging failed, too many characters or formatting error");
}

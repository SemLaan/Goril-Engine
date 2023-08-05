// This code only implements platform.h if the platform is windows
// Otherwise this entire script does nothing
#ifdef __win__
#include "../core/platform.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "windows.h"


namespace gr
{

	b8 initialize_platform()
	{
		return true;
	}

	void shutdown_platform()
	{
	}

	static u8 log_level_colors[MAX_LOG_LEVELS] =
	{
		192,
		4,
		6,
		9,
		2,
		8
	};
	
	void platform_log_message(log_level level, const char* message)
	{
		static HANDLE hConsole = {};
		if (!hConsole)
		{
			hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		}
		SetConsoleTextAttribute(hConsole, log_level_colors[level]);
		OutputDebugStringA(message);
		printf(message);
	}
}
#endif
// This code only implements platform.h if the platform is windows
// Otherwise this entire script does nothing
#ifdef __win__
#include "../core/platform.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "core/logger.h"


namespace GR
{
	// Forward declaring window callbacks
	//LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	size_t GetPlatformRequiredMemory()
	{
		return 0;
	}

	b8 InitializePlatform()
	{
		WNDCLASSEX wc = {};

		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = NULL;
		wc.lpfnWndProc = DefWindowProc; /// TODO: custom window proc
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetModuleHandle(NULL);
		wc.hIcon = LoadIcon(NULL, IDC_ICON);
		wc.hIconSm = NULL;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName = L"beef";
		wc.lpszClassName = L"meat";

		RegisterClassEx(&wc);

		HWND hwnd = CreateWindowEx(
			NULL, L"meat", L"Stront window",/// TODO: ask the application for a window name
			WS_OVERLAPPEDWINDOW, 
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, GetModuleHandle(NULL), NULL
		);

		if (hwnd == NULL)
		{
			GRFATAL("Creating window failed");
			return false;
		}

		ShowWindow(hwnd, SW_SHOW);

		/*
		MSG msg = { };
		while (GetMessage(&msg, NULL, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		*/

		return true;
	}

	void ShutdownPlatform()
	{
	}

	static u8 logLevelColors[MAX_LOG_LEVELS] =
	{
		192,
		4,
		6,
		9,
		2,
		8
	};
	
	void PlatformLogMessage(log_level level, const char* message)
	{
		static HANDLE hConsole = {};
		if (!hConsole)
		{
			hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		}
		SetConsoleTextAttribute(hConsole, logLevelColors[level]);
		OutputDebugStringA(message);
		printf(message);
	}
}
#endif
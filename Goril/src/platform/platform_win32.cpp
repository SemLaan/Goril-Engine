// This code only implements platform.h if the platform is windows
// Otherwise this entire script does nothing
#ifdef __win__
#include "../core/platform.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "core/logger.h"
#include "core/asserts.h"


namespace GR
{

	// Forward declaring window callbacks
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	b8 InitializePlatform()
	{
		const wchar_t* menuName = L"gorilwinmenu";
		const wchar_t* className = L"gorilwinclass";

		WNDCLASSEX wc = {};

		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = NULL;
		wc.lpfnWndProc = WindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetModuleHandle(NULL);
		wc.hIcon = LoadIcon(NULL, IDC_ICON);
		wc.hIconSm = NULL;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName = menuName;
		wc.lpszClassName = className;

		RegisterClassEx(&wc);

		state->hwnd = CreateWindowEx(
			NULL, className, L"Stront window",/// TODO: ask the application for a window name
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

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}
#endif
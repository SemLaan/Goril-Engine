// This code only implements platform.h if the platform is windows
// Otherwise this entire script does nothing
#ifdef __win__
#include "../core/platform.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "core/logger.h"
#include "core/asserts.h"
#include "core/gr_memory.h"
#include "core/event.h"


namespace GR
{

	struct PlatformState
	{
		HWND hwnd;
	};

	static PlatformState* state = nullptr;

	// Forward declaring window callbacks
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	b8 InitializePlatform(const wchar_t* windowName, b8 startMinimized)
	{
		GRASSERT_DEBUG(state == nullptr); // If this fails init platform was called twice
		GRINFO("Initializing platform subsystem...");
		state = (PlatformState*)GetSubsysBumpAllocator()->Alloc(sizeof(PlatformState), MEM_TAG_PLATFORM_SUBSYS);
		Zero(state, sizeof(PlatformState));

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

		u32 windowStyle = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
		if (startMinimized)
			windowStyle |= WS_MINIMIZE;

		state->hwnd = CreateWindowEx(
			NULL, className, windowName,
			windowStyle,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, GetModuleHandle(NULL), NULL
		);

		if (state->hwnd == NULL)
		{
			GRFATAL("Creating window failed");
			GetSubsysBumpAllocator()->Free(state);
			state = nullptr;
			return false;
		}

		ShowWindow(state->hwnd, SW_SHOW);

		return true;
	}

	void ShutdownPlatform()
	{
		if (state == nullptr) 
		{
			GRINFO("Platform startup failed, skipping shutdown");
			return;
		}
		else 
		{
			GRINFO("Shutting down platform subsystem...");
		}

		GetSubsysBumpAllocator()->Free(state);
	}

	void PlatformProcessMessage()
	{
		MSG msg = {};

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
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
	
	void PlatformLogString(log_level level, const char* message)
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
		switch (uMsg)
		{
		case WM_DESTROY:
		{
			EventData data = {};
			InvokeEvent(EVCODE_QUIT, data);
			PostQuitMessage(0);
			return 0;
		}
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}
}
#endif
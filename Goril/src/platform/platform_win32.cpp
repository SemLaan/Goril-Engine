// This code only implements platform.h if the platform is windows
// Otherwise this entire script does nothing
#ifdef __win__
// Both of these headers are implemented here
#include "core/platform.h"
#include "renderer/vulkan/vulkan_platform.h" 

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <vulkan/vulkan_win32.h>
#include "core/logger.h"
#include "core/asserts.h"
#include "core/gr_memory.h"
#include "core/event.h"
#include "core/input.h"
#include <stdio.h>



struct PlatformState
{
	HWND hwnd;
	u32 width;
	u32 height;
	u32 xPosPreMaximize, yPosPreMaximize;
	u32 widthPreMaximize, heightPreMaximize;
	DWORD windowStyle;
	DWORD windowExStyle;
	bool fullscreenKeyDown;
	bool fullscreenActive;
};

static PlatformState* state = nullptr;

// Forward declaring window callbacks
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool InitializePlatform(const wchar_t* windowName)
{
	GRASSERT_DEBUG(state == nullptr); // If this fails init platform was called twice
	GRINFO("Initializing platform subsystem...");
	state = (PlatformState*)Alloc(GetGlobalAllocator(), sizeof(PlatformState), MEM_TAG_PLATFORM_SUBSYS);
	Zero(state, sizeof(PlatformState));

	const wchar_t* menuName = L"gorilwinmenu";
	const wchar_t* className = L"gorilwinclass";

	WNDCLASSEX wc{};

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

	state->windowStyle = WS_OVERLAPPEDWINDOW;
	state->windowExStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW;

	state->hwnd = CreateWindowEx(
		state->windowExStyle, className, windowName,
		state->windowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, GetModuleHandle(NULL), NULL
	);

	if (state->hwnd == NULL)
	{
		GRFATAL("Creating window failed");
		Free(GetGlobalAllocator(), state);
		state = nullptr;
		return false;
	}

	ShowWindow(state->hwnd, SW_SHOW);

	RECT clientRect;
	if (GetClientRect(state->hwnd, &clientRect))
	{
		state->width = clientRect.right - clientRect.left;
		state->height = clientRect.bottom - clientRect.top;
	}

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

	Free(GetGlobalAllocator(), state);
}

void PlatformProcessMessage()
{
	MSG msg{};

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
	static HANDLE hConsole{};
	if (!hConsole)
	{
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		GRINFO("Console initialized");
	}
	SetConsoleTextAttribute(hConsole, logLevelColors[level]);
	OutputDebugStringA(message);
	printf(message);
}

vec2i GetPlatformWindowSize()
{
	return { (i32)state->width, (i32)state->height };
}

void SetMousePosition(vec2i position)
{
	if (state->fullscreenActive)
	{
		SetCursorPos(position.x, position.y);
	}
	else
	{
		RECT windowRect, clientRect;
		if (!GetWindowRect(state->hwnd, &windowRect) || !GetClientRect(state->hwnd, &clientRect))
		{
			GRASSERT(false);
		}
		i32 leftBorderSize = ((windowRect.right - windowRect.left) - clientRect.right) / 2;
		i32 topBorderSize = ((windowRect.bottom - windowRect.top) - clientRect.bottom) / 2;
		SetCursorPos(windowRect.left + leftBorderSize + position.x, windowRect.top + GetSystemMetrics(SM_CYCAPTION) + leftBorderSize + position.y);
	}
}

void SetWindowTitle(const wchar_t* windowName)
{
	SetWindowTextW(state->hwnd, windowName);
}

void ToggleFullscreen()
{
	SetFullscreen(!state->fullscreenActive);
}

void SetFullscreen(bool enabled)
{
	if (enabled == state->fullscreenActive)
		return;

	state->fullscreenActive = enabled;

	if (enabled)
	{
		SetWindowLongW(state->hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowLongW(state->hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW);

		HMONITOR monitor = MonitorFromWindow(state->hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFOEXW monitorInfo{};
		monitorInfo.cbSize = sizeof(monitorInfo);
		if (GetMonitorInfoW(monitor, &monitorInfo))
		{
			RECT windowRect;
			if (GetWindowRect(state->hwnd, &windowRect))
			{
				state->xPosPreMaximize = windowRect.left;
				state->yPosPreMaximize = windowRect.top;
			}
			state->widthPreMaximize = state->width;
			state->heightPreMaximize = state->height;
			state->width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
			state->height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

			SetWindowPos(state->hwnd, nullptr,
				monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.top,
				monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
				SWP_NOZORDER
			);

			EventData eventData{};
			eventData.u32[0] = state->width;
			eventData.u32[1] = state->height;
			InvokeEvent(EVCODE_WINDOW_RESIZED, eventData);
		}
	}
	else
	{
		SetWindowLongW(state->hwnd, GWL_STYLE, state->windowStyle);
		SetWindowLongW(state->hwnd, GWL_EXSTYLE, state->windowExStyle);

		ShowWindow(state->hwnd, SW_SHOW);

		SetWindowPos(state->hwnd, nullptr,
			state->xPosPreMaximize,
			state->yPosPreMaximize,
			state->widthPreMaximize,
			state->heightPreMaximize,
			NULL
		);

		state->width = state->widthPreMaximize;
		state->height = state->heightPreMaximize;

		EventData eventData{};
		eventData.u32[0] = state->width;
		eventData.u32[1] = state->height;
		InvokeEvent(EVCODE_WINDOW_RESIZED, eventData);
	}
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
	{
		EventData data{};
		InvokeEvent(EVCODE_QUIT, data);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		// ======= Input processing ===============
	case WM_MOUSEMOVE:
	{
		i32 xPos = GET_X_LPARAM(lParam);
		i32 yPos = GET_Y_LPARAM(lParam);
		ProcessMouseMove(xPos, yPos);
		break;
	}
	case WM_LBUTTONDOWN:
		ProcessButton(true, BUTTON_LEFTMOUSEBTN);
		break;
	case WM_LBUTTONUP:
		ProcessButton(false, BUTTON_LEFTMOUSEBTN);
		break;
	case WM_MBUTTONDOWN:
		ProcessButton(true, BUTTON_MIDMOUSEBTN);
		break;
	case WM_MBUTTONUP:
		ProcessButton(false, BUTTON_MIDMOUSEBTN);
		break;
	case WM_RBUTTONDOWN:
		ProcessButton(true, BUTTON_RIGHTMOUSEBTN);
		break;
	case WM_RBUTTONUP:
		ProcessButton(false, BUTTON_RIGHTMOUSEBTN);
		break;
	case WM_KEYDOWN:
		if (wParam == 0x12)
			break;
		ProcessKey(true, (KeyCode)wParam);
		break;
	case WM_KEYUP:
		if (wParam == 0x12)
			break;
		ProcessKey(false, (KeyCode)wParam);
		break;
		/// TODO: process syskeys for input, if you want to suffer
	case WM_SIZE:
	{
		RECT clientRect;
		u32 width, height;
		if (GetClientRect(hwnd, &clientRect))
		{
			width = clientRect.right - clientRect.left;
			height = clientRect.bottom - clientRect.top;
			if (width != state->width || height != state->height)
			{
				GRDEBUG("Window resized");
				state->width = width;
				state->height = height;
				EventData eventData{};
				eventData.u32[0] = state->width;
				eventData.u32[1] = state->height;
				InvokeEvent(EVCODE_WINDOW_RESIZED, eventData);
				break;
			}
		}
	}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// ============ Vulkan platform implementation =======================
void GetPlatformExtensions(void** extensionNamesDarray)
{
	const char* vk_khr_win32_surface_extensionname = "VK_KHR_win32_surface";
	extensionNamesDarray = (void**)DarrayPushback(extensionNamesDarray, &vk_khr_win32_surface_extensionname);
}

bool PlatformCreateSurface(VkInstance instance, VkAllocationCallbacks* allocator, VkSurfaceKHR* out_surface)
{
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.hinstance = GetModuleHandle(NULL);
	createInfo.hwnd = state->hwnd;

	if (VK_SUCCESS != vkCreateWin32SurfaceKHR(instance, &createInfo, allocator, out_surface))
	{
		Zero(out_surface, sizeof(VkSurfaceKHR));
		return false;
	}

	return true;
}

#endif
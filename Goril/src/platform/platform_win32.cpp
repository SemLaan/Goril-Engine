// This code only implements platform.h if the platform is windows
// Otherwise this entire script does nothing
#ifdef __win__
// Both of these headers are implemented here
#include "core/platform.h"
#include "rendering/vulkan/vulkan_platform.h" 

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
			/// TODO: process syskeys, if you want to suffer
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	// ============ Vulkan platform implementation =======================
	void GetPlatformExtensions(Darray<const void*>* extensionNames)
	{
		extensionNames->Pushback(&"VK_KHR_win32_surface");
	}

	b8 PlatformCreateSurface(VkInstance instance, VkAllocationCallbacks* allocator, VkSurfaceKHR* out_surface)
	{
		VkWin32SurfaceCreateInfoKHR createInfo = {};
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
}
#endif
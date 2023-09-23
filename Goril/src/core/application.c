#include "application.h"

#include "logger.h"
#include "platform/platform.h"
#include "gr_memory.h"
#include "event.h"
#include "input.h"
#include "renderer/renderer.h"
#include "timer.h"
#include <stdio.h>

static bool appRunning = false;
static bool appSuspended = false;

static f64 previousFrameTime = 0.f;
/// NOTE: temporary
static f64 previousFrameTimes[1000] = {};
static u32 frameIndex = 0;

static bool OnQuit(EventCode type, EventData data);
static bool OnResize(EventCode type, EventData data);
static bool OnKeyDown(EventCode type, EventData data);

bool InitializeEngine(GameConfig config)
{
	size_t engineMemoryRequirement = MiB;
	size_t subsysAllocatorRequirement = KiB * 5;

	// Initialize subsystems
	if (!InitializeMemory(config.game_instance_memory_requirement + engineMemoryRequirement, subsysAllocatorRequirement))
	{
		GRFATAL("Memory failed to initialize");
		return false;
	}
	if (!InitializeEvent())
	{
		GRFATAL("Event system failed to initialize");
		return false;
	}
	if (!InitializeInput())
	{
		GRFATAL("Input system failed to initialize");
		return false;
	}
	if (!InitializePlatform(config.windowTitle))
	{
		GRFATAL("Platform failed to initialize");
		return false;
	}
	if (!InitializeRenderer())
	{
		GRFATAL("Renderer system failed to initialize");
		return false;
	}

	RegisterEventListener(EVCODE_QUIT, OnQuit);
	RegisterEventListener(EVCODE_WINDOW_RESIZED, OnResize);
	RegisterEventListener(EVCODE_KEY_DOWN, OnKeyDown);

	StartOrResetTimer(&g_timer);

	appRunning = true;
	return true;
}

bool RunEngine(GameFunctions* gameFunctions)
{
	gameFunctions->GameInit();

	while (appRunning)
	{
		f64 currentFrameTime = TimerSecondsSinceStart(g_timer);
		g_deltaTime = currentFrameTime - previousFrameTime;
		previousFrameTime = currentFrameTime;

		/// NOTE: temporary
		previousFrameTimes[frameIndex] = g_deltaTime;
		frameIndex = (frameIndex + 1) % 1000;

		f64 framerateCalculation = 0;
		for (u32 i = 0; i < 1000; ++i)
		{
			framerateCalculation += previousFrameTimes[i];
		}
		framerateCalculation /= 1000.f;

		const u32 max_window_title_chars = 100;
		char windowTitle[max_window_title_chars];
		snprintf(windowTitle, max_window_title_chars, "%.0f", 1.f / framerateCalculation);
		SetWindowTitle(windowTitle);

		UpdateInput();
		PlatformProcessMessage(); /// TODO: sleep platform every loop if app suspended to not waste pc resources
		if (!appSuspended)
		{
			gameFunctions->GameUpdate();
			gameFunctions->GameRender();
			RenderFrame();
			if (GetKeyDown(KEY_ESCAPE))
				appRunning = false;
		}
	}

	UnregisterEventListener(EVCODE_QUIT, OnQuit);
	UnregisterEventListener(EVCODE_WINDOW_RESIZED, OnResize);
	UnregisterEventListener(EVCODE_KEY_DOWN, OnKeyDown);
	gameFunctions->GameShutdown();

	return true;
}

void ShutdownEngine()
{
	GRINFO("Application ran for: %.2fs", TimerSecondsSinceStart(g_timer));

	// Shutdown subsystems
	ShutdownRenderer();
	ShutdownPlatform();
	ShutdownInput();
	ShutdownEvent();
	ShutdownMemory();

	WriteLogsToFile();
}

static bool OnQuit(EventCode type, EventData data)
{
	appRunning = false;
	return false;
}

static bool OnResize(EventCode type, EventData data)
{
	if (data.u32[0] == 0 || data.u32[1] == 0)
	{
		appSuspended = true;
		GRINFO("App suspended");
	}
	else if (appSuspended)
	{
		appSuspended = false;
		GRINFO("App unsuspended");
	}
	return false;
}

static bool OnKeyDown(EventCode type, EventData data)
{
	if (data.u8[0] == KEY_F11)
		ToggleFullscreen();
	return false;
}

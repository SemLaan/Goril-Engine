#include "application.h"

#include "logger.h"
#include "platform.h"
#include "gr_memory.h"
#include "event.h"
#include "input.h"
#include "renderer/renderer.h"
#include "timer.h"

#include <string>

namespace GR
{
	static b8 appRunning = false;
	static b8 appSuspended = false;

	static f64 previousFrameTime{};
	/// NOTE: temporary
	static f64 previousFrameTimes[1000]{};
	static u32 frameIndex = 0;

	static b8 OnQuit(EventCode type, EventData data);
	static b8 OnResize(EventCode type, EventData data);
	static b8 OnKeyDown(EventCode type, EventData data);

	b8 InitializeEngine(GameConfig config)
	{
		g_timer = CreateAndStartTimer();

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
		appRunning = true;
		return true;
	}

	b8 RunEngine(GorilGame* gameInstance)
	{
		gameInstance->Init();

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

			SetWindowTitle(std::format(L"{:.0f}", 1.f / framerateCalculation).c_str());
			
			UpdateInput();
			PlatformProcessMessage(); /// TODO: sleep platform every loop if app suspended to not waste pc resources
			if (!appSuspended)
			{
				gameInstance->Update();
				b8 succesfull = BeginFrame();
				if (succesfull)
				{
					gameInstance->Render();
					EndFrame();
				}
				if (GetKeyDown(KEY_ESCAPE))
					appRunning = false;
			}
		}

		UnregisterEventListener(EVCODE_QUIT, OnQuit);
		UnregisterEventListener(EVCODE_WINDOW_RESIZED, OnResize);
		UnregisterEventListener(EVCODE_KEY_DOWN, OnKeyDown);
		gameInstance->Shutdown();

		return true;
	}

	void ShutdownEngine()
	{
		// Shutdown subsystems
		ShutdownRenderer();
		ShutdownPlatform();
		ShutdownInput();
		ShutdownEvent();
		ShutdownMemory();

		GRINFO("Application ran for: {:.2f}s", TimerSecondsSinceStart(g_timer));
		WriteLogsToFile();
	}

	static b8 OnQuit(EventCode type, EventData data)
	{
		appRunning = false;
		return false;
	}

	static b8 OnResize(EventCode type, EventData data)
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

	static b8 OnKeyDown(EventCode type, EventData data)
	{
		if (data.u8[0] == KEY_F11)
			ToggleFullscreen();
		return false;
	}
}
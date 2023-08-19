#include "application.h"

#include "logger.h"
#include "platform.h"
#include "gr_memory.h"
#include "event.h"
#include "input.h"
#include "rendering/renderer.h"

namespace GR
{
	static b8 appRunning = false;
	static b8 appSuspended = false;

	b8 OnQuit(EventCode type, EventData data);
	b8 OnResize(EventCode type, EventData data);


	b8 InitializeEngine(GameConfig config)
	{
		size_t engineMemoryRequirement = MiB;
		size_t subsysAllocatorRequirement = KiB * 5;

		// Initialize subsystems
		if (!InitializeMemory(config.game_instance_memory_requirement + engineMemoryRequirement, subsysAllocatorRequirement))
		{
			GRFATAL("Memory failed to initialize");
			return false;
		}
		if (!InitializeLogger())
		{
			GRFATAL("Logger failed to initialize");
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
		appRunning = true;
		return true;
	}

	b8 RunEngine(GorilGame* gameInstance)
	{
		gameInstance->Init();

		while (appRunning)
		{
			UpdateInput();
			PlatformProcessMessage(); /// TODO: sleep platform every loop if app suspended to not waste pc resources
			if (!appSuspended)
			{
				gameInstance->Update();
				b8 succesfull = UpdateRenderer();
				if (succesfull)
					gameInstance->Render();
				if (GetKeyDown(KEY_ESCAPE))
					appRunning = false;
			}
		}

		UnregisterEventListener(EVCODE_QUIT, OnQuit);
		UnregisterEventListener(EVCODE_WINDOW_RESIZED, OnResize);
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
		ShutdownLogger();
		ShutdownMemory();
	}

	b8 OnQuit(EventCode type, EventData data)
	{
		appRunning = false;
		return false;
	}

	b8 OnResize(EventCode type, EventData data)
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
}
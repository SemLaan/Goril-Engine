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

	b8 OnQuit(EventCode type, EventData data)
	{
		appRunning = false;
		return false;
	}

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
		if (!InitializePlatform(config.windowTitle, config.startMinimized))
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
		appRunning = true;
		return true;
	}

	b8 RunEngine(GorilGame* gameInstance)
	{
		gameInstance->Init();

		while (appRunning)
		{
			UpdateInput();
			PlatformProcessMessage();
			gameInstance->Update();
			b8 succesfull = UpdateRenderer();
			if (succesfull)
				gameInstance->Render();
			if (GetKeyDown(KEY_ESCAPE))
				appRunning = false;
		}

		UnregisterEventListener(EVCODE_QUIT, OnQuit);
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
}
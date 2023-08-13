#include "application.h"

#include "logger.h"
#include "platform.h"
#include "gr_memory.h"
#include "event.h"

namespace GR
{
	static b8 appRunning = false;

	b8 OnQuit(EventType type, EventData data)
	{
		appRunning = false;
		return false;
	}

	b8 InitializeEngine(GameConfig config)
	{
		size_t engineMemoryRequirement = KiB * 5;
		size_t subsysAllocatorRequirement = KiB;

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
		if (!InitializePlatform(config.windowTitle, config.startMinimized))
		{
			GRFATAL("Platform failed to initialize");
			return false;
		}

		RegisterEventListener(EVCODE_QUIT, OnQuit);
		appRunning = true;
		return true;
	}

	b8 RunEngine(GorilGame* gameInstance)
	{
		if (!appRunning)
			return true;
		gameInstance->Init();

		while (appRunning)
		{
			PlatformProcessMessage();
			gameInstance->Update();
			gameInstance->Render();
		}

		UnregisterEventListener(EVCODE_QUIT, OnQuit);
		gameInstance->Shutdown();

		return true;
	}

	void ShutdownEngine()
	{
		// Shutdown subsystems
		ShutdownPlatform();
		ShutdownEvent();
		ShutdownLogger();
		ShutdownMemory();
	}
}
#pragma once
#include "defines.h"


namespace GR
{

	struct EventData
	{
		// 128 bytes of data layed out in a union of different data types
		union
		{
			u32 uint32[4];
		};
	};

	enum EventType
	{
		EVCODE_QUIT,
		MAX_EVENTS
	};

	typedef b8 (*PFN_OnEvent)(EventType type, EventData data);

	b8 InitializeEvent();

	void ShutdownEvent();

	GRAPI void RegisterEventListener(EventType type, PFN_OnEvent listener);
	GRAPI void UnregisterEventListener(EventType type, PFN_OnEvent listener);

	GRAPI void InvokeEvent(EventType type, EventData data);
}

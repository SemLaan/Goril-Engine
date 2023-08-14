#pragma once
#include "defines.h"


namespace GR
{

	struct EventData
	{
		// 128 bytes of data layed out in a union of different data types
		union
		{
			u8 u8[16];
			i8 i8[16];
			b8 b8[16];

			u16 u16[8];
			i16 i16[8];

			u32 u32[4];
			i32 i32[4];
			f32 f32[4];
			b32 b32[4];

			u64 u64[2];
			i64 i64[2];
			f64 f64[2];
		};
	};

	enum EventType
	{ /// NOTE: always add new event types just before MAX_EVENTS
		EVCODE_QUIT,
		EVCODE_TEST,
		EVCODE_KEY_DOWN,
		EVCODE_KEY_UP,
		EVCODE_BUTTON_DOWN,
		EVCODE_BUTTON_UP,
		EVCODE_MOUSE_MOVED,
		MAX_EVENTS
	};

	typedef b8 (*PFN_OnEvent)(EventType type, EventData data);

	b8 InitializeEvent();

	void ShutdownEvent();

	GRAPI void RegisterEventListener(EventType type, PFN_OnEvent listener);
	GRAPI void UnregisterEventListener(EventType type, PFN_OnEvent listener);

	GRAPI void InvokeEvent(EventType type, EventData data);
}

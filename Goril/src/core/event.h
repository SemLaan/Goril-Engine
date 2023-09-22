#pragma once
#include "defines.h"



typedef struct EventData
{
	// 128 bytes of data layed out in a union of different data types
	union
	{
		u8 u8[16];
		i8 i8[16];
		bool b8[16];

		u32 u32[4];
		i32 i32[4];
		f32 f32[4];
		b32 b32[4];

		u64 u64[2];
		i64 i64[2];
		f64 f64[2];
	};
} EventData;

typedef enum EventCode
{ /// NOTE: always add new event types just before MAX_EVENTS
	EVCODE_QUIT,
	EVCODE_TEST,
	EVCODE_KEY_DOWN,
	EVCODE_KEY_UP,
	EVCODE_BUTTON_DOWN,
	EVCODE_BUTTON_UP,
	EVCODE_MOUSE_MOVED,
	EVCODE_WINDOW_RESIZED,
	MAX_EVENTS
} EventCode;

typedef bool(*PFN_OnEvent)(EventCode type, EventData data);

bool InitializeEvent();

void ShutdownEvent();

void RegisterEventListener(EventCode type, PFN_OnEvent listener);
void UnregisterEventListener(EventCode type, PFN_OnEvent listener);

void InvokeEvent(EventCode type, EventData data);


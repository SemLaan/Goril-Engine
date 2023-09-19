#include "event.h"
#include "containers/darray.h"
#include "core/gr_memory.h"
#include "core/asserts.h"



struct EventState
{
	PFN_OnEvent* eventCallbacksDarrays[MAX_EVENTS];
};

static EventState* state = nullptr;

b8 InitializeEvent()
{
	GRASSERT_DEBUG(state == nullptr); // If this triggers init got called twice
	GRINFO("Initializing event subsystem...");
	state = (EventState*)GetGlobalAllocator()->Alloc(sizeof(EventState), MEM_TAG_EVENT_SUBSYS);
	Zero(state, sizeof(EventState));

	return true;
}

void ShutdownEvent()
{
	if (state == nullptr)
	{
		GRINFO("Events startup failed, skipping shutdown");
		return;
	}
	else
	{
		GRINFO("Shutting down events subsystem...");
	}

	for (PFN_OnEvent* callbackDarray : state->eventCallbacksDarrays)
	{
		if (callbackDarray)
		{
			DarrayDestroy(callbackDarray);
		}
	}
	GetGlobalAllocator()->Free(state);
}

void RegisterEventListener(EventCode type, PFN_OnEvent listener)
{
	if (!state->eventCallbacksDarrays[type])
	{
		state->eventCallbacksDarrays[type] = (PFN_OnEvent*)DarrayCreate(sizeof(PFN_OnEvent), 5, GetGlobalAllocator(), MEM_TAG_EVENT_SUBSYS);
	}

#ifndef GR_DIST
	for (u32 i = 0; i < DarrayGetSize(state->eventCallbacksDarrays[type]); ++i)
	{
		GRASSERT_MSG(state->eventCallbacksDarrays[type][i] != listener, "Tried to insert duplicate listener");
	}
#endif // !GR_DIST

	state->eventCallbacksDarrays[type] = (PFN_OnEvent*)DarrayPushback(state->eventCallbacksDarrays[type], &listener);
}

void UnregisterEventListener(EventCode type, PFN_OnEvent listener)
{
	GRASSERT_DEBUG(state->eventCallbacksDarrays[type]);

	for (u32 i = 0; i < DarrayGetSize(state->eventCallbacksDarrays[type]); ++i)
	{
		if (state->eventCallbacksDarrays[type][i] == listener)
		{
			DarrayPopAt(state->eventCallbacksDarrays[type], i);
			return;
		}
	}

	GRASSERT_MSG(false, "Tried to remove listener that isn't listening");
}

void InvokeEvent(EventCode type, EventData data)
{
	if (state->eventCallbacksDarrays[type])
	{
		for (u32 i = 0; i < DarrayGetSize(state->eventCallbacksDarrays[type]); ++i)
		{
			// PFN_OnEvent callbacks return true if the event is handled so then we don't need to call anything else
			if (state->eventCallbacksDarrays[type][i](type, data))
				return;
		}
	}
}

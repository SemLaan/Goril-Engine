#include "event.h"
#include "containers/darray.h"
#include "core/gr_memory.h"

namespace GR
{

	struct EventState
	{
		Darray<PFN_OnEvent> eventCallbacks[MAX_EVENTS];
	};

	static EventState* state;

	b8 InitializeEvent()
	{
		state = (EventState*)GetSubsysBumpAllocator()->Alloc(sizeof(EventState), MEM_TAG_EVENT_SUBSYS);
		Zero(state, sizeof(EventState));

		return true;
	}

	void ShutdownEvent()
	{
		for (Darray<PFN_OnEvent>& callbackDarray : state->eventCallbacks)
		{
			if (callbackDarray.GetRawElements())
			{
				callbackDarray.Deinitialize();
			}
		}
		GetSubsysBumpAllocator()->Free(state);
	}

	void RegisterEventListener(EventType type, PFN_OnEvent listener)
	{
		if (!state->eventCallbacks[type].GetRawElements())
		{
			state->eventCallbacks[type].Initialize(MEM_TAG_EVENT_SUBSYS, 5);
		}

#ifndef GR_DIST
		for (u32 i = 0; i < state->eventCallbacks[type].Size(); ++i)
		{
			GRASSERT_MSG(state->eventCallbacks[type][i] != listener, "Tried to insert duplicate listener");
		}
#endif // !GR_DIST

		state->eventCallbacks[type].Pushback(listener);
	}

	void UnregisterEventListener(EventType type, PFN_OnEvent listener)
	{
		GRASSERT_DEBUG(state->eventCallbacks[type].GetRawElements());

		for (u32 i = 0; i < state->eventCallbacks[type].Size(); ++i)
		{
			if (state->eventCallbacks[type][i] == listener)
			{
				state->eventCallbacks[type].PopAt(i);
				return;
			}
		}

		GRASSERT_MSG(false, "Tried to remove listener that isn't listening");
	}

	void InvokeEvent(EventType type, EventData data)
	{
		if (state->eventCallbacks[type].GetRawElements())
		{
			for (u32 i = 0; i < state->eventCallbacks[type].Size(); ++i)
			{
				// PFN_OnEvent callbacks return true if the event is handled so then we don't need to call anything else
				if (state->eventCallbacks[type][i](type, data))
					return;
			}
		}
	}
}
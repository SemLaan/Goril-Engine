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
		GetSubsysBumpAllocator()->Free(state);
	}

	void RegisterEventListener(EventType type, PFN_OnEvent listener)
	{
		if (!state->eventCallbacks[type].GetRawElements())
		{
			state->eventCallbacks[type].Initialize(MEM_TAG_EVENT_SUBSYS, 5);
		}

		/// TODO: check for duplicate listeners if not in dist

		state->eventCallbacks[type].Pushback(listener);
	}

	void UnregisterEventListener(EventType type, PFN_OnEvent listener)
	{
		///TODO: unregister event
	}

	void InvokeEvent(EventType type, EventData data)
	{
		if (state->eventCallbacks[type].GetRawElements())
		{
			for (u32 i = 0; i < state->eventCallbacks[type].Size(); ++i)
			{
				state->eventCallbacks[type][i](type, data);
			}
		}
	}
}
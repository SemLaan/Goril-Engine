#include "input.h"

#include "core/asserts.h"
#include "core/gr_memory.h"
#include "core/event.h"

namespace GR
{
	struct InputState
	{
		b8 keyStates[256];
		b8 previousKeyStates[256];
		b8 buttonStates[7];
		b8 previousButtonStates[7];
	};

	static InputState* state = nullptr;


	b8 InitializeInput()
	{
		GRASSERT_DEBUG(state == nullptr); // If this fails it means init was called twice
		GRINFO("Initializing input subsystem...");

		state = (InputState*)GetSubsysBumpAllocator()->Alloc(sizeof(InputState), MEM_TAG_INPUT_SUBSYS);
		Zero(state, sizeof(InputState));

		return true;
	}

	void ShutdownInput()
	{
		if (state == nullptr)
		{
			GRINFO("Input startup failed, skipping shutdown");
			return;
		}
		else
		{
			GRINFO("Shutting down input subsystem...");
		}

		GetSubsysBumpAllocator()->Free(state);
	}

	void UpdateInput()
	{
		MemCopy(&state->previousKeyStates, &state->keyStates, sizeof(state->keyStates));
		MemCopy(&state->previousButtonStates, &state->buttonStates, sizeof(state->buttonStates));
	}

	bool GetKeyDown(KeyCode key)
	{
		return state->keyStates[key];
	}

	bool GetKeyDownPrevious(KeyCode key)
	{
		return state->previousKeyStates[key];
	}

	void ProcessKey(b8 down, KeyCode key)
	{
		if (state->keyStates[key] != down)
		{
			state->keyStates[key] = down;
			EventData data{};
			data.u8[0] = key;
			if (down)
				InvokeEvent(EVCODE_KEY_DOWN, data);
			else
				InvokeEvent(EVCODE_KEY_UP, data);
		}
	}

	void ProcessButton(b8 down, ButtonCode button)
	{

	}

	void ProcessMouseMove(float x, float y)
	{

	}
}
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
		i32 mousePosX;
		i32 mousePosY;
		i32 previousMousePosX;
		i32 previousMousePosY;
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
		state->previousMousePosX = state->mousePosX;
		state->previousMousePosY = state->mousePosY;
	}

	b8 GetKeyDown(KeyCode key)
	{
		return state->keyStates[key];
	}

	b8 GetKeyDownPrevious(KeyCode key)
	{
		return state->previousKeyStates[key];
	}

	b8 GetButtonDown(ButtonCode button)
	{
		return state->buttonStates[button];
	}

	b8 GetButtonDownPrevious(ButtonCode button)
	{
		return state->previousButtonStates[button];
	}

	void GetMousePos(i32* x, i32* y)
	{
		*x = state->mousePosX;
		*y = state->mousePosY;
	}

	void GetMousePosPrevious(i32* x, i32* y)
	{
		*x = state->previousMousePosX;
		*y = state->previousMousePosY;
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
		if (state->buttonStates[button] != down)
		{
			state->buttonStates[button] = down;
			EventData data{};
			data.u8[0] = button;
			if (down)
				InvokeEvent(EVCODE_BUTTON_DOWN, data);
			else
				InvokeEvent(EVCODE_BUTTON_UP, data);
		}
	}

	void ProcessMouseMove(i32 x, i32 y)
	{
		if (state->mousePosX != x || state->mousePosY != y)
		{
			state->mousePosX = x;
			state->mousePosY = y;
			EventData data{};
			data.i32[0] = state->mousePosX;
			data.i32[1] = state->mousePosY;
			data.i32[2] = state->previousMousePosX;
			data.i32[3] = state->previousMousePosY;
			InvokeEvent(EVCODE_MOUSE_MOVED, data);
		}
	}
}
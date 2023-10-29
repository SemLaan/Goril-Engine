#pragma once
#include <core/goril_game.h>
#include <renderer/buffer.h>
#include <renderer/texture.h>
#include "math/lin_alg.h"


typedef struct GameState
{
	Texture texture;
	Texture texture2;
	vec3 camPosition;
	vec3 camRotation;
	mat4 view;
	mat4 proj;
	mat4 perspective;
	mat4 orthographic;
	bool mouseEnabled;
	bool perspectiveEnabled;
} GameState;

extern GameState* gamestate;


bool Init();
bool Update();
bool Render();
bool Shutdown();

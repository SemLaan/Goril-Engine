#pragma once
#include <core/goril_game.h>
#include <renderer/buffer.h>
#include <renderer/texture.h>
#include "math/lin_alg.h"


typedef struct GameState
{
	Texture texture;
	vec3 camPosition;
	mat4 camRotation;
	mat4 view;
	mat4 proj;
} GameState;

extern GameState* gamestate;


bool Init();
bool Update();
bool Shutdown();

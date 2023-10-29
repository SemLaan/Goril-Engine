#pragma once
#include <core/goril_game.h>
#include <renderer/buffer.h>
#include <renderer/texture.h>
#include <renderer/camera.h>
#include "math/lin_alg.h"


typedef struct GameState
{
	Texture texture;
	Camera camera;
} GameState;

extern GameState* gamestate;


bool Init();
bool Update();
bool Shutdown();

#pragma once
#include <core/goril_game.h>
#include <renderer/buffer.h>
#include <renderer/texture.h>
#include <renderer/camera.h>
#include "math/lin_alg.h"


typedef struct Button
{
    vec2 position;
    vec2 size;
} Button;

typedef struct GameState
{
	Texture texture;
	Camera camera;
	Button buttons[2];
} GameState;

extern GameState* gamestate;


bool Init();
bool Update();
bool Shutdown();

#pragma once
#include <core/goril_game.h>
#include <renderer/buffer.h>
#include <renderer/texture.h>
#include "math/lin_alg.h"


typedef struct GameState
{
	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;
	Texture texture;
	vec3 camPosition;
	vec3 camRotation;
	mat4 view;
	mat4 proj;
	mat4 perspective;
	mat4 orthographic;
	bool mouseEnabled;
	bool perspectiveEnabled;
	f64 meshOneActive;
} GameState;

extern GameState* gamestate;


bool Init();
bool Update();
bool Render();
bool Shutdown();

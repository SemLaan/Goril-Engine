#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <core/goril_game.h>
#include <glm/glm.hpp>
#include <renderer/buffer.h>
#include <renderer/texture.h>



typedef struct GameState
{
	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;
	Texture texture;
	glm::vec3 camPosition;
	glm::vec3 camRotation;
	glm::mat4 view;
	glm::mat4 proj;
	bool mouseEnabled;
} GameState;

extern GameState* gamestate;


bool Init();
bool Update();
bool Render();
bool Shutdown();

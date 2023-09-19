#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <core/goril_game.h>
#include <glm/glm.hpp>
#include <renderer/buffer.h>
#include <renderer/texture.h>



class Game : public GorilGame
{
private:
	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;
	Texture texture;
	glm::vec3 camPosition;
	glm::vec3 camRotation;
	glm::mat4 view;
	glm::mat4 proj;
	bool mouseEnabled;

public:
	// Inherited via GorilGame
	bool Init() override;
	bool Update() override;
	bool Render() override;
	bool Shutdown() override;
};
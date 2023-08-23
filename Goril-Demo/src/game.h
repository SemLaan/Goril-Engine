#pragma once
#include <core/goril_game.h>
#include <glm/glm.hpp>
#include <renderer/buffer.h>


using namespace GR;


class Game : public GR::GorilGame
{
private:
	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;
	glm::vec3 camPosition;
	glm::vec3 camRotation;
	glm::mat4 view;
	glm::mat4 proj;
	b8 mouseEnabled;

public:
	// Inherited via GorilGame
	b8 Init() override;
	b8 Update() override;
	b8 Render() override;
	b8 Shutdown() override;
};
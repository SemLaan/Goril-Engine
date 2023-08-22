#pragma once
#include <core/goril_game.h>
#include <glm/glm.hpp>
#include <rendering/buffer.h>


using namespace GR;


class Game : public GR::GorilGame
{
private:
	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;

public:
	// Inherited via GorilGame
	b8 Init() override;
	b8 Update() override;
	b8 Render() override;
	b8 Shutdown() override;
};
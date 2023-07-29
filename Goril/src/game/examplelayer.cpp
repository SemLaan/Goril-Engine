#include "examplelayer.h"
#include <iostream>
#include <glm/glm.hpp>
#include "goril.h"

using namespace Goril;
using namespace Goril::LLR;

ExampleLayer::ExampleLayer()
{
}

void ExampleLayer::OnAttach()
{
	
}

void ExampleLayer::OnDetach()
{
}

void ExampleLayer::UpdateLayer(float deltaTime)
{

	struct Vertex
	{
		Vertex(float x, float y)
		{
			pos = glm::vec2(x, y);
		}
		glm::vec2 pos;
	};

	unsigned int* indices = new unsigned int[3] {0, 1, 2};
	Ref<IndexBuffer> ib = IndexBuffer::Create(indices, 3);

	Vertex* vertices = new Vertex[3]{ {0, 0}, {0, 1}, {1, 0} };
	Ref<VertexBuffer> vb = VertexBuffer::Create(vertices, 3 * sizeof(Vertex));
	vb->SetBufferLayout({ ShaderDataType::Vec2F });

	Ref<VertexArray> va = VertexArray::Create();
	va->SetIndexBuffer(ib);
	va->AddVertexBuffer(vb);


	RenderCommand::SetClearColor(1, 1, 0, 0);
	RenderCommand::Clear(COLOR_BUFFER);

	RenderCommand::DrawIndexed(va, nullptr);


	delete[] indices;
	delete[] vertices;

	std::cout << deltaTime << "\n";
	glm::vec3 beef = glm::vec3(1, 2, 3);
	std::cout << glm::dot(beef, glm::vec3(2, 2, 4)) << "\n";
}

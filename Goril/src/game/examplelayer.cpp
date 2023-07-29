#include "examplelayer.h"
#include <iostream>
#include <glm/glm.hpp>
#include "goril.h"

using namespace Goril;

ExampleLayer::ExampleLayer()
{
}

void ExampleLayer::OnAttach()
{
	unsigned int* beef = new unsigned int[3] {1, 4, 3};
	Ref<LLR::IndexBuffer> test = LLR::IndexBuffer::Create(beef, 10);
}

void ExampleLayer::OnDetach()
{
}

void ExampleLayer::UpdateLayer(float deltaTime)
{
	std::cout << deltaTime << "\n";
	glm::vec3 beef = glm::vec3(1, 2, 3);
	std::cout << glm::dot(beef, glm::vec3(2, 2, 4)) << "\n";
}

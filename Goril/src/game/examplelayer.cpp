#include "examplelayer.h"
#include <iostream>
#include <glm/glm.hpp>

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
	std::cout << deltaTime << "\n";
	glm::vec3 beef = glm::vec3(1, 2, 3);
	std::cout << glm::dot(beef, glm::vec3(2, 2, 4)) << "\n";
}

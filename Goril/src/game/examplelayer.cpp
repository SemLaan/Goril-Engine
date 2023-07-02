#include "examplelayer.h"
#include <iostream>

ExampleLayer::ExampleLayer()
	: m_test(0)
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
	std::cout << deltaTime << std::endl;
	m_test++;
}

#pragma once
#include <goril.h>

class ExampleLayer : public Goril::Layer
{
public:
	ExampleLayer();
	~ExampleLayer() = default;

	// Inherited via Layer
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void UpdateLayer(float deltaTime) override;

private:

};
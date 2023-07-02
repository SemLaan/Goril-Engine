#pragma once

namespace Goril
{

	class Layer
	{
	public:
		Layer();
		virtual ~Layer() = default;

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void UpdateLayer(float deltaTime) = 0;

		Layer& operator=(const Layer&) = delete;
		Layer(const Layer&) = delete;
		Layer& operator=(Layer&&) = delete;
		Layer(Layer&&) = delete;
	};
}
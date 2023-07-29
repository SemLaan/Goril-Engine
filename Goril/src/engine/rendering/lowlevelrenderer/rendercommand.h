#pragma once
#include "rendererapi.h"

namespace Goril::LLR
{

	class RenderCommand
	{
	private:
		static Scope<RendererAPI> s_rendererAPI;
	public:
		inline static void Init()
		{
			s_rendererAPI->Init();
		}

		inline static void Shutdown()
		{
			s_rendererAPI->Shutdown();
		}

		inline static void SetClearColor(float r, float g, float b, float a)
		{
			s_rendererAPI->SetClearColor(r, g, b, a);
		}

		inline static void Clear(ClearOption clearOptions)
		{
			s_rendererAPI->Clear(clearOptions);
		}

		inline static void SetBlendFunction(BlendOption source, BlendOption destination)
		{
			s_rendererAPI->SetBlendFunction(source, destination);
		}

		inline static void SetStencilFunc(StencilOption func, int reference, unsigned int mask)
		{
			s_rendererAPI->SetStencilFunc(func, reference, mask);
		}

		inline static void SetStencilMask(unsigned int mask)
		{
			s_rendererAPI->SetStencilMask(mask);
		}

		inline static void SetStencilOp(StencilOption fail, StencilOption zfail, StencilOption zpass)
		{
			s_rendererAPI->SetStencilOp(fail, zfail, zpass);
		}

		inline static void EnableBlend(bool enable)
		{
			s_rendererAPI->EnableBlend(enable);
		}

		inline static void EnableDepthTest(bool enable)
		{
			s_rendererAPI->EnableDepthTest(enable);
		}

		inline static void EnableStencilTest(bool enable)
		{
			s_rendererAPI->EnableStencilTest(enable);
		}

		inline static void DrawIndexed(const Ref<VertexArray> vertexArray, const Ref<Shader> shader, unsigned int indexCount = 0)
		{
			s_rendererAPI->DrawIndexed(vertexArray, shader, indexCount);
		}

		inline static void DrawIndexedInstanced(const Ref<VertexArray> vertexArray, const Ref<Shader> shader, unsigned int instanceCount, unsigned int indexCount = 0)
		{
			s_rendererAPI->DrawIndexedInstanced(vertexArray, shader, instanceCount, indexCount);
		}
	};
}
#pragma once
#include "llrenums.h"
#include "core/gorilmem.h"

namespace Goril::LLR
{
	class RendererAPI
	{
	private:
		static API s_API;
	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void SetClearColor(float r, float g, float b, float a) = 0;
		virtual void Clear(ClearOption clearOptions) = 0;
		virtual void SetBlendFunction(BlendOption source, BlendOption destination) = 0;
		virtual void SetStencilFunc(StencilOption func, int reference, unsigned int mask) = 0;
		virtual void SetStencilMask(unsigned int mask) = 0;
		virtual void SetStencilOp(StencilOption fail, StencilOption zfail, StencilOption zpass) = 0;
		virtual void EnableBlend(bool enable) = 0;
		virtual void EnableDepthTest(bool enable) = 0;
		virtual void EnableStencilTest(bool enable) = 0;
		virtual int GetDrawCallsAndReset() const = 0;

		/// <summary>
		/// Draw call for one non-instanced object (automatically gets the vertex and index buffer from the vertex array)
		/// </summary>
		/// <param name="vertexArray"></param>
		/// <param name="shader"></param>
		/// <param name="indexCount"></param>
		//virtual void DrawIndexed(const std::shared_ptr<VertexArray> vertexArray, const std::shared_ptr<Shader> shader, unsigned int indexCount = 0) = 0;

		/// <summary>
		/// Draw call for instanced objects (automatically gets the vertex and index buffer from the vertex array)
		/// </summary>
		/// <param name="vertexArray"></param>
		/// <param name="shader"></param>
		/// <param name="indexCount"></param>
		//virtual void DrawIndexedInstanced(const std::shared_ptr<VertexArray> vertexArray, const std::shared_ptr<Shader> shader, unsigned int instanceCount, unsigned int indexCount = 0) = 0;

		/// <returns>The API type that is currently being used not the api object itself</returns>
		static API GetAPI() { return s_API; }
		static Scope<RendererAPI> Create();
	};
}
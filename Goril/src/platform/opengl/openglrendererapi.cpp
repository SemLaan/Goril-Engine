#include "openglrendererapi.h"
#include "openglerrormanagement.h"
#include <glad/glad.h>
#include "openglvertexarray.h"
#include "openglindexbuffer.h"
#include "openglshader.h"

namespace Goril::LLR::OpenGL
{

	static GLenum BlendOptionToOpenGLBlendOption(BlendOption blendOption)
	{
		switch (blendOption)
		{
		case BlendOption::ZERO:						return GL_ZERO;
		case BlendOption::ONE:							return GL_ONE;
		case BlendOption::SRC_COLOR:					return GL_SRC_COLOR;
		case BlendOption::ONE_MINUS_SRC_COLOR:			return GL_ONE_MINUS_SRC_COLOR;
		case BlendOption::DST_COLOR:					return GL_DST_COLOR;
		case BlendOption::ONE_MINUS_DST_COLOR:			return GL_ONE_MINUS_DST_COLOR;
		case BlendOption::SRC_ALPHA:					return GL_SRC_ALPHA;
		case BlendOption::ONE_MINUS_SRC_ALPHA:			return GL_ONE_MINUS_SRC_ALPHA;
		case BlendOption::DST_ALPHA:					return GL_DST_ALPHA;
		case BlendOption::ONE_MINUS_DST_ALPHA:			return GL_ONE_MINUS_DST_ALPHA;
		case BlendOption::CONSTANT_COLOR:				return GL_CONSTANT_COLOR;
		case BlendOption::ONE_MINUS_CONSTANT_COLOR:	return GL_ONE_MINUS_CONSTANT_COLOR;
		case BlendOption::CONSTANT_ALPHA:				return GL_CONSTANT_ALPHA;
		case BlendOption::ONE_MINUS_CONSTANT_ALPHA:	return GL_ONE_MINUS_CONSTANT_ALPHA;
		}
		return 0;
	}

	static GLenum StencilOptionToOpenGLStencilOption(StencilOption blendOption)
	{
		switch (blendOption)
		{
		case StencilOption::NEVER:		return GL_NEVER;
		case StencilOption::INCR:		return GL_INCR;
		case StencilOption::KEEP:		return GL_KEEP;
		case StencilOption::DECR:		return GL_DECR;
		case StencilOption::EQUAL:		return GL_EQUAL;
		case StencilOption::ALWAYS:		return GL_ALWAYS;
		}
		return 0;
	}

	void OpenGLRendererAPI::Init()
	{
	}

	void OpenGLRendererAPI::Shutdown()
	{
	}

	void OpenGLRendererAPI::SetClearColor(float r, float g, float b, float a)
	{
		G(glClearColor(r, g, b, a));
	}

	void OpenGLRendererAPI::Clear(ClearOption clearOptions)
	{
		unsigned int options = 0;
		if (clearOptions & ClearOption::COLOR_BUFFER)
			options = options | GL_COLOR_BUFFER_BIT;
		if (clearOptions & ClearOption::DEPTH_BUFFER)
			options = options | GL_DEPTH_BUFFER_BIT;
		if (clearOptions & ClearOption::STENCIL_BUFFER)
		{
			options = options | GL_STENCIL_BUFFER_BIT;
			glStencilMask(0xff);
		}
		G(glClear(options));
	}

	void OpenGLRendererAPI::SetBlendFunction(BlendOption source, BlendOption destination)
	{
		G(glBlendFunc(BlendOptionToOpenGLBlendOption(source), BlendOptionToOpenGLBlendOption(destination)));
	}

	void OpenGLRendererAPI::SetStencilFunc(StencilOption func, int reference, unsigned int mask)
	{
		G(glStencilFunc(StencilOptionToOpenGLStencilOption(func), reference, mask));
	}

	void OpenGLRendererAPI::SetStencilMask(unsigned int mask)
	{
		G(glStencilMask(mask));
	}

	void OpenGLRendererAPI::SetStencilOp(StencilOption fail, StencilOption zfail, StencilOption zpass)
	{
		G(glStencilOp(StencilOptionToOpenGLStencilOption(fail), StencilOptionToOpenGLStencilOption(zfail), StencilOptionToOpenGLStencilOption(zpass)));
	}

	void OpenGLRendererAPI::EnableBlend(bool enable)
	{
		if (enable)
		{
			G(glEnable(GL_BLEND));
		}
		else
		{
			G(glDisable(GL_BLEND));
		}
	}

	void OpenGLRendererAPI::EnableDepthTest(bool enable)
	{
		if (enable)
		{
			G(glEnable(GL_DEPTH_TEST));
		}
		else
		{
			G(glDisable(GL_DEPTH_TEST));
		}
	}

	void OpenGLRendererAPI::EnableStencilTest(bool enable)
	{
		if (enable)
		{
			G(glEnable(GL_STENCIL_TEST));
		}
		else
		{
			G(glDisable(GL_STENCIL_TEST));
		}
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray> vertexArray, const Ref<Shader> shader, unsigned int indexCount)
	{
		std::dynamic_pointer_cast<OpenGLShader>(shader)->Bind();
		std::dynamic_pointer_cast<OpenGLVertexArray>(vertexArray)->Bind();
		std::dynamic_pointer_cast<OpenGLIndexBuffer>(vertexArray->GetIndexBuffer())->Bind();
		unsigned int count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		G(glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr));
	}

	void OpenGLRendererAPI::DrawIndexedInstanced(const Ref<VertexArray> vertexArray, const Ref<Shader> shader, unsigned int instanceCount, unsigned int indexCount)
	{
		std::dynamic_pointer_cast<OpenGLShader>(shader)->Bind();
		std::dynamic_pointer_cast<OpenGLVertexArray>(vertexArray)->Bind();
		std::dynamic_pointer_cast<OpenGLIndexBuffer>(vertexArray->GetIndexBuffer())->Bind();
		unsigned int count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		G(glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr, instanceCount));
	}
}
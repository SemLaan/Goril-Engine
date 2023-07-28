#include "openglrendererapi.h"


namespace Goril::LLR::OpenGL
{

	void OpenGLRendererAPI::Init()
	{
	}

	void OpenGLRendererAPI::Shutdown()
	{
	}

	void OpenGLRendererAPI::SetClearColor(float r, float g, float b, float a)
	{
	}

	void OpenGLRendererAPI::Clear(ClearOption clearOptions)
	{
	}

	void OpenGLRendererAPI::SetBlendFunction(BlendOption source, BlendOption destination)
	{
	}

	void OpenGLRendererAPI::SetStencilFunc(StencilOption func, int reference, unsigned int mask)
	{
	}

	void OpenGLRendererAPI::SetStencilMask(unsigned int mask)
	{
	}

	void OpenGLRendererAPI::SetStencilOp(StencilOption fail, StencilOption zfail, StencilOption zpass)
	{
	}

	void OpenGLRendererAPI::EnableBlend(bool enable)
	{
	}

	void OpenGLRendererAPI::EnableDepthTest(bool enable)
	{
	}

	void OpenGLRendererAPI::EnableStencilTest(bool enable)
	{

	}

	int OpenGLRendererAPI::GetDrawCallsAndReset() const
	{
		return 0;
	}
}
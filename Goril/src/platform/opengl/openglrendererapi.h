#pragma once
#include "rendering/lowlevelrenderer/rendererapi.h"

namespace Goril::LLR
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		void Init() override;
		void Shutdown() override;

		int GetDrawCallsAndReset() const override;

		void SetClearColor(float r, float g, float b, float a) override;
		void Clear(ClearOption clearOptions) override;
		void SetBlendFunction(BlendOption source, BlendOption destination) override;
		void SetStencilFunc(StencilOption func, int reference, unsigned int mask) override;
		void SetStencilMask(unsigned int mask) override;
		void SetStencilOp(StencilOption fail, StencilOption zfail, StencilOption zpass) override;
		void EnableBlend(bool enable) override;
		void EnableDepthTest(bool enable) override;
		void EnableStencilTest(bool enable) override;
	};
}
#include "rendercommand.h"


namespace Goril::LLR
{
	Scope<RendererAPI> RenderCommand::s_rendererAPI = RendererAPI::Create();
}
#include "shader.h"
#include "opengl/openglshader.h"
#include <fstream>
#include <sstream>
#include "rendererapi.h"

namespace Goril::LLR
{

	std::string Shader::ParseShader(const std::string& filepath)
	{
		
		std::ifstream stream(filepath);
		std::string line;
		std::stringstream ss;

		while (getline(stream, line))
		{
			ss << line << "\n";
		}
		return ss.str();
	}

	Ref<Shader> Shader::Create(const std::string& vertexShaderCode, const std::string& fragmentShaderCode)
	{
		switch (RendererAPI::GetAPIType())
		{
		case API::OpenGL:
			return CreateRef<OpenGL::OpenGLShader>(vertexShaderCode, fragmentShaderCode);
		default:
			return nullptr;
		}
	}
}


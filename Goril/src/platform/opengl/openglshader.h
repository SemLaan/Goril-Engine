#pragma once
#include "rendering/lowlevelrenderer/shader.h"
#include <string>
#include <unordered_map>

namespace Goril::LLR::OpenGL
{

	class OpenGLShader : public Shader
	{
	private:
		unsigned int m_rendererID;
		std::unordered_map<std::string, int> m_uniformLocationCache;

		int GetUniformLocation(const std::string name);
		unsigned int CompileShader(unsigned int type, const char* source);
		unsigned int CreateShaderProgram(unsigned int vertexShader, unsigned int fragmentShader);

	public:
		OpenGLShader(const std::string& vertexShaderCode, const std::string& fragmentShaderCode);
		~OpenGLShader();

		void Bind() const;

		void SetInt(const std::string& name, int value) override;
		void SetFloat(const std::string& name, float value) override;
		void SetVec2F(const std::string& name, const glm::vec2& value) override;
		void SetVec3F(const std::string& name, const glm::vec3& value) override;
		void SetVec4F(const std::string& name, const glm::vec4& value) override;
		void SetMat2(const std::string& name, const glm::mat2& mat) override;
		void SetMat3(const std::string& name, const glm::mat3& mat) override;
		void SetMat4(const std::string& name, const glm::mat4& mat) override;
	};
}
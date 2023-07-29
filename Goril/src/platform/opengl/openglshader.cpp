#include "openglshader.h"
#include "openglerrormanagement.h"
#include "glad/glad.h"
#include <iostream>

namespace Goril::LLR::OpenGL
{
	OpenGLShader::OpenGLShader(const std::string& vertexShaderCode, const std::string& fragmentShaderCode)
	{
		unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShaderCode.c_str());
		unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderCode.c_str());
		m_rendererID = CreateShaderProgram(vs, fs);
	}

	OpenGLShader::~OpenGLShader()
	{
		G(glDeleteProgram(m_rendererID));
	}

	unsigned int OpenGLShader::CompileShader(unsigned int type, const char* source)
	{
		unsigned int id = glCreateShader(type);
		const char* src = source; // source.c_str();
		G(glShaderSource(id, 1, &src, nullptr));
		G(glCompileShader(id));

		int result;
		G(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
		if (result == GL_FALSE)
		{
			int length;
			G(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
			char* message = (char*)malloc(length * sizeof(char));
			G(glGetShaderInfoLog(id, length, &length, message));
			std::cout << "Failed to compile shader: " << std::endl << message << std::endl;
			free(message);
		}
		return id;
	}

	unsigned int OpenGLShader::CreateShaderProgram(unsigned int vertexShader, unsigned int fragmentShader)
	{
		unsigned int program = glCreateProgram();

		G(glAttachShader(program, vertexShader));
		G(glAttachShader(program, fragmentShader));
		G(glLinkProgram(program));

		int success;
		char infoLog[512];
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(program, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		G(glValidateProgram(program));

		G(glDeleteShader(vertexShader));
		G(glDeleteShader(fragmentShader));

		return program;
	}

	void OpenGLShader::Bind() const
	{
		G(glUseProgram(m_rendererID));
	}


	// --------------------------------- Uniform setting ----------------------------------------
	int OpenGLShader::GetUniformLocation(const std::string name)
	{
		if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end())
			return m_uniformLocationCache[name];

		G(int location = glGetUniformLocation(m_rendererID, name.c_str()));
		if (location == -1)
		{
			std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
			CUSTOMWARNING("at line: ");
		}

		m_uniformLocationCache[name] = location; // cache found location
		return location;
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		Bind();
		G(glUniform1i(GetUniformLocation(name), value));
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		Bind();
		G(glUniform1f(GetUniformLocation(name), value));
	}

	void OpenGLShader::SetVec2F(const std::string& name, const glm::vec2& value)
	{
		Bind();
		G(glUniform2f(GetUniformLocation(name), value.x, value.y));
	}

	void OpenGLShader::SetVec3F(const std::string& name, const glm::vec3& value)
	{
		Bind();
		G(glUniform3f(GetUniformLocation(name), value.x, value.y, value.z));
	}

	void OpenGLShader::SetVec4F(const std::string& name, const glm::vec4& value)
	{
		Bind();
		G(glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w));
	}

	void OpenGLShader::SetMat2(const std::string& name, const glm::mat2& mat)
	{
		Bind();
		G(glUniformMatrix2fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]));
	}

	void OpenGLShader::SetMat3(const std::string& name, const glm::mat3& mat)
	{
		Bind();
		G(glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]));
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& mat)
	{
		Bind();
		G(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]));
	}
}
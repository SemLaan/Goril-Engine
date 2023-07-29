#pragma once
#include "llrenums.h"
#include "core/gorilmem.h"
#include <string>
#include <glm/glm.hpp>

namespace Goril::LLR
{
	class Shader
	{
	protected:
		/// <summary>
		/// Reads in the shader code from the given file.
		/// </summary>
		/// <param name="filepath"></param>
		/// <returns>The shader in string form</returns>
		static std::string ParseShader(const std::string& filepath);
	public:
		virtual ~Shader() = default;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetVec2F(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetVec3F(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetVec4F(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat3(const std::string& name, const glm::mat3& mat) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& mat) = 0;

		virtual const std::string& GetName() const = 0;

		/// <summary>
		/// Creates a shader based on the shader code in the given file.
		/// </summary>
		/// <param name="filepath"></param>
		/// <returns>A smart pointer to the shader object.</returns>
		static Scope<Shader> Create(const std::string& filepath);
	};
}
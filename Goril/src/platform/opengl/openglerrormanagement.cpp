#include "OpenGLErrorManagement.h"
#include <glad/glad.h>
#include <fstream>
#include <iostream>

namespace Goril::LLR::OpenGL
{

	void glfw_error_callback(int error, const char* description)
	{
		fprintf(stderr, "Glfw Error %d: %s\n", error, description);
	}

	void glClearError()
	{
		while (glGetError() != GL_NO_ERROR);
	}

	bool glLogCall(const char* function, const char* file, int line)
	{
		while (GLenum error = glGetError())
		{
			std::cout << "[OpenGL ERROR]: (" << error << "): " << function << " " << file << " " << line << std::endl;
			return false;
		}
		return true;
	}

}
#pragma once
#include "rendering/lowlevelrenderer/indexbuffer.h"

namespace Goril::LLR::OpenGL
{
	class OpenGLIndexBuffer : public IndexBuffer
	{
	private:
		unsigned int m_rendererID; //just an id for the buffer
		unsigned int m_count;
	public:
		OpenGLIndexBuffer(unsigned int count);
		OpenGLIndexBuffer(const unsigned int* pData, unsigned int count);
		~OpenGLIndexBuffer();

		void Bind() const;

		// Inherited via IndexBuffer
		void SetBufferData(const unsigned int* pData, Range insertRange) override;
		inline unsigned int GetCount() const override { return m_count; }
	};
}
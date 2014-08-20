#include "GLES2/gl2.h"
#include "gltexture.h"

GLTexture::GLTexture(int width, int height, GLint format, const void* data)
{
	width_ = width;
	height_ = height;
	format_ = format;
	framebuffer_id_ = 0;

	glGenTextures(1, &id_);
	glBindTexture(GL_TEXTURE_2D, id_);
	glTexImage2D(GL_TEXTURE_2D, 0, format_, width_, height_, 0, (GLenum)format_, GL_UNSIGNED_BYTE, data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool GLTexture::GenerateFramebuffer()
{
	glGenFramebuffers(1, &framebuffer_id_);	
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id_, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
 return true;
}

void GLTexture::SetPixels(const void* data)
{
	glBindTexture(GL_TEXTURE_2D, id_);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, format_, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

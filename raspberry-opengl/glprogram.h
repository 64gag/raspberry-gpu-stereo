#ifndef GLPROGRAM_H_
#define GLPROGRAM_H_

#include "glshader.h"

class GLProgram
{
	GLShader *vertex_shader_;
	GLShader *fragment_shader_;
	GLuint id_;
	GLint texel_location_;
public:
	GLProgram() : vertex_shader_(0), fragment_shader_(0), id_(0), texel_location_(0) {}
	GLProgram(GLShader* vertex_shader, GLShader* fragment_shader);
	~GLProgram() {}

	GLuint id() { return id_; }
	GLint texel_location() { return texel_location_; }
	void set_texel_location(GLint l) {texel_location_ = l; }
};

#endif /* GLPROGRAM_H_ */

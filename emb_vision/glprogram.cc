#include <iostream>
#include "GLES2/gl2.h"
#include "glprogram.h"
#include "glshader.h"

#define DEBUG_SHADERS 0

GLProgram::GLProgram(GLShader* vertex_shader, GLShader* fragment_shader)
{
	vertex_shader_ = vertex_shader;
	fragment_shader_ = fragment_shader;
	id_ = glCreateProgram();
	glAttachShader(id_, vertex_shader_->id());
	glAttachShader(id_, fragment_shader_->id());
	glLinkProgram(id_);

	#if DEBUG_SHADERS
		char log[1024];
		glGetProgramInfoLog(id_, sizeof log, 0, log);
		std::cout << log;
		std::cout << "Program " << id_ << " from " << vertex_shader_->id() << " and " << fragment_shader_->id() << std::endl;
	#endif
}

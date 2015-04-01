#ifndef GLSHADER_H_
#define GLSHADER_H_

#include <iostream>

class GLShader
{
	GLuint id_;
	GLuint type_;
	std::string source_code_;
public:
	GLShader() : id_(0), type_(0) {}
	~GLShader() {}
	GLShader(const char* name, GLuint type);

	GLuint id() { return id_; }
};

#endif /* GLSHADDER_H_ */

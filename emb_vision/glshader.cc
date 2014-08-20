#include <iostream>
#include <fstream>
#include <sstream>
#include "GLES2/gl2.h"
#include "glshader.h"

#define DEBUG_SHADERS 0

/* printShaderInfoLog
 * From OpenGL Shading Language 3rd Edition, p215-216
 * Display (hopefully) useful error messages if shader fails to compile */
void printShaderInfoLog(GLint shader)
{
	int infoLogLen = 0;
	int charsWritten = 0;
	GLchar *infoLog;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);

	if (infoLogLen > 0){
		infoLog = new GLchar[infoLogLen];
		/* error check for fail to allocate memory omitted */
		glGetShaderInfoLog(shader, infoLogLen, &charsWritten, infoLog);
		std::cout << "InfoLog : " << std::endl << infoLog << std::endl;
		delete [] infoLog;
	}
}

GLShader::GLShader(const char* name, GLuint type)
{
	std::string filename("shader/");
	filename += type == GL_VERTEX_SHADER ? "vs/" : "fs/";
	filename += name;
	filename += ".glsl";

	std::ifstream in;
	std::stringstream shader_code;
	in.open(filename.c_str(), std::ifstream::in);
	shader_code << in.rdbuf();
	in.close();

	/* Now create and compile the shader */
	type_ = type;
	id_ = glCreateShader(type_);
	source_code_ = shader_code.str();
	glShaderSource(id_, 1, (const GLchar**)(&source_code_), 0);
	glCompileShader(id_);

	/* Compilation check */
	GLint compiled;
	glGetShaderiv(id_, GL_COMPILE_STATUS, &compiled);
	if(compiled==0){
		#if DEBUG_SHADERS
			std::cout << "Failed to compile shader " << filename << std::endl;
			printShaderInfoLog(id_);
		#endif
		glDeleteShader(id_);
	}else{
		#if DEBUG_SHADERS
			std::cout << "Good compilation of " << filename << " : " << id_ << std::endl;
			std::cout << source_code_ << std::endl;
		#endif
	}
}

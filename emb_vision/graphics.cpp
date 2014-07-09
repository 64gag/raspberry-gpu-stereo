/*
 Thank Chris Cummings for this class, not me. I modified the functions (slightly! I actually liked them) I directly worked with because I am a bit obsessive and deleted some code I was not going to use.
*/

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include "bcm_host.h"
#include "graphics.h"

#define glCheck() assert(glGetError() == 0)
#define DEBUG_SHADERS 1

uint32_t GScreenWidth;
uint32_t GScreenHeight;
EGLDisplay GDisplay;
EGLSurface GSurface;
EGLContext GContext;

const char *shader_files[SHAD_COUNT] = {
 "shaders/s_iraw.glsl", "shaders/s_henc.glsl", "shaders/s_swin.glsl",
 "shaders/s_dmap.glsl", "shaders/s_hdec.glsl", "shaders/s_vrtx.glsl"
};

GfxShader GFrag[SHAD_COUNT];
GfxProgram GProg[SHAD_COUNT - 1];
GLuint GQuadVertexBuffer;

void InitGraphics()
{
	bcm_host_init();
	int32_t success = 0;
	EGLBoolean result;
	EGLint num_config;

	static EGL_DISPMANX_WINDOW_T nativewindow;

	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T dispman_update;
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;

	static const EGLint attribute_list[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};

	static const EGLint context_attributes[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	EGLConfig config;

	// get an EGL display connection
	GDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(GDisplay!=EGL_NO_DISPLAY);
	glCheck();

	// initialize the EGL display connection
	result = eglInitialize(GDisplay, NULL, NULL);
	assert(EGL_FALSE != result);
	glCheck();

	// get an appropriate EGL frame buffer configuration
	result = eglChooseConfig(GDisplay, attribute_list, &config, 1, &num_config);
	assert(EGL_FALSE != result);
	glCheck();

	// get an appropriate EGL frame buffer configuration
	result = eglBindAPI(EGL_OPENGL_ES_API);
	assert(EGL_FALSE != result);
	glCheck();

	// create an EGL rendering context
	GContext = eglCreateContext(GDisplay, config, EGL_NO_CONTEXT, context_attributes);
	assert(GContext!=EGL_NO_CONTEXT);
	glCheck();

	// create an EGL window surface
	success = graphics_get_display_size(0 /* LCD */, &GScreenWidth, &GScreenHeight);
	assert( success >= 0 );

	dst_rect.x = 0;
	dst_rect.y = 0;
	dst_rect.width = GScreenWidth;
	dst_rect.height = GScreenHeight;

	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.width = GScreenWidth << 16;
	src_rect.height = GScreenHeight << 16;        

	dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
	dispman_update = vc_dispmanx_update_start( 0 );

	dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
		0/*layer*/, &dst_rect, 0/*src*/,
		&src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, (DISPMANX_TRANSFORM_T)0/*transform*/);

	nativewindow.element = dispman_element;
	nativewindow.width = GScreenWidth;
	nativewindow.height = GScreenHeight;
	vc_dispmanx_update_submit_sync( dispman_update );

	glCheck();

	GSurface = eglCreateWindowSurface( GDisplay, config, &nativewindow, NULL );
	assert(GSurface != EGL_NO_SURFACE);
	glCheck();

	// connect the context to the surface
	result = eglMakeCurrent(GDisplay, GSurface, GSurface, GContext);
	assert(EGL_FALSE != result);
	glCheck();

	// Set background color and clear buffers
	glClearColor(0.15f, 0.25f, 0.35f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT );

	GFrag[SHAD_VRTX].LoadVertexShader(shader_files[SHAD_VRTX]);
	for(int p = 0; p < SHAD_COUNT - 1; p++){
		GFrag[p].LoadFragmentShader(shader_files[p]);
		GProg[p].Create(&GFrag[SHAD_VRTX],&GFrag[p]);
	}

	glCheck();

	//create an ickle vertex buffer
	static const GLfloat quad_vertex_positions[] = {
		0.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};
	glGenBuffers(1, &GQuadVertexBuffer);
	glCheck();
	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertex_positions), quad_vertex_positions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glCheck();
}

void BeginFrame()
{
	// Prepare viewport
	glViewport ( 0, 0, GScreenWidth, GScreenHeight );
	glCheck();

	// Clear the background
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glCheck();
}

void EndFrame()
{
	eglSwapBuffers(GDisplay,GSurface);
	glCheck();
}

void ReleaseGraphics()
{

}

// printShaderInfoLog
// From OpenGL Shading Language 3rd Edition, p215-216
// Display (hopefully) useful error messages if shader fails to compile
void printShaderInfoLog(GLint shader)
{
	int infoLogLen = 0;
	int charsWritten = 0;
	GLchar *infoLog;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);

	if (infoLogLen > 0){
		infoLog = new GLchar[infoLogLen];
		// error check for fail to allocate memory omitted
		glGetShaderInfoLog(shader, infoLogLen, &charsWritten, infoLog);
		std::cout << "InfoLog : " << std::endl << infoLog << std::endl;
		delete [] infoLog;
	}
}

bool GfxShader::LoadVertexShader(const char* filename)
{
	//cheeky bit of code to read the whole file into memory
	assert(!Src);
	FILE* f = fopen(filename, "rb");
	assert(f);
	fseek(f,0,SEEK_END);
	int sz = ftell(f);
	fseek(f,0,SEEK_SET);
	Src = new GLchar[sz+1];
	fread(Src,1,sz,f);
	Src[sz] = 0; //null terminate it!
	fclose(f);

	//now create and compile the shader
	GlShaderType = GL_VERTEX_SHADER;
	Id = glCreateShader(GlShaderType);
	glShaderSource(Id, 1, (const GLchar**)&Src, 0);
	glCompileShader(Id);
	glCheck();

	//compilation check
	GLint compiled;
	glGetShaderiv(Id, GL_COMPILE_STATUS, &compiled);
	if(compiled==0)
	{
		#if DEBUG_SHADERS
			printf("Failed to compile vertex shader %s:\n%s\n", filename, Src);
			printShaderInfoLog(Id);
		#endif
		glDeleteShader(Id);
		return false;
	}else{
		#if DEBUG_SHADERS
			printf("Compiled vertex shader %s:\n%s\n", filename, Src);
		#endif
	}

	return true;
}

bool GfxShader::LoadFragmentShader(const char* filename)
{
	//cheeky bit of code to read the whole file into memory
	assert(!Src);
	FILE* f = fopen(filename, "rb");
	assert(f);
	fseek(f,0,SEEK_END);
	int sz = ftell(f);
	fseek(f,0,SEEK_SET);
	Src = new GLchar[sz+1];
	fread(Src,1,sz,f);
	Src[sz] = 0; //null terminate it!
	fclose(f);

	//now create and compile the shader
	GlShaderType = GL_FRAGMENT_SHADER;
	Id = glCreateShader(GlShaderType);
	glShaderSource(Id, 1, (const GLchar**)&Src, 0);
	glCompileShader(Id);
	glCheck();

	//compilation check
	GLint compiled;
	glGetShaderiv(Id, GL_COMPILE_STATUS, &compiled);
	if(compiled==0) {
		#if DEBUG_SHADERS
			printf("Failed to compile fragment shader %s:\n%s\n", filename, Src);
			printShaderInfoLog(Id);
		#endif
		glDeleteShader(Id);
		return false;
	} else {
		#if DEBUG_SHADERS
			printf("Compiled fragment shader %s:\n%s\n", filename, Src);
		#endif
	}

	return true;
}

bool GfxProgram::Create(GfxShader* vertex_shader, GfxShader* fragment_shader)
{
	VertexShader = vertex_shader;
	FragmentShader = fragment_shader;
	Id = glCreateProgram();
	glAttachShader(Id, VertexShader->GetId());
	glAttachShader(Id, FragmentShader->GetId());
	glLinkProgram(Id);
	glCheck();

	#if DEBUG_SHADERS
		printf("Created program id %d from vs %d and fs %d\n", GetId(), VertexShader->GetId(), FragmentShader->GetId());

		// Prints the information log for a program object
		char log[1024];
		glGetProgramInfoLog(Id,sizeof log,NULL,log);
		printf("%d:program:\n%s\n", Id, log);
	#endif
	return true;	
}

bool GfxTexture::CreateRGBA(int width, int height, const void* data)
{
	Width = width;
	Height = height;
	glGenTextures(1, &Id);
	glCheck();
	glBindTexture(GL_TEXTURE_2D, Id);
	glCheck();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glCheck();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);
	glCheck();
	glBindTexture(GL_TEXTURE_2D, 0);
	IsRGBA = true;
	return true;
}

bool GfxTexture::CreateGreyScale(int width, int height, const void* data)
{
	Width = width;
	Height = height;
	glGenTextures(1, &Id);
	glCheck();
	glBindTexture(GL_TEXTURE_2D, Id);
	glCheck();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, Width, Height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
	glCheck();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);
	glCheck();
	glBindTexture(GL_TEXTURE_2D, 0);
	IsRGBA = false;
	return true;
}

bool GfxTexture::GenerateFrameBuffer()
{
	//Create a frame buffer that points to this texture
	glGenFramebuffers(1,&FramebufferId);
	glCheck();
	glBindFramebuffer(GL_FRAMEBUFFER,FramebufferId);
	glCheck();
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,Id,0);
	glCheck();
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	glCheck();
	return true;
}

void GfxTexture::SetPixels(const void* data)
{
	glBindTexture(GL_TEXTURE_2D, Id);
	glCheck();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width, Height, IsRGBA ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
	glCheck();
	glBindTexture(GL_TEXTURE_2D, 0);
	glCheck();
}

void DrawTextureRect(GfxTexture* texture, float x0, float y0, float x1, float y1, GfxTexture* render_target, shader_enum_t prog_id)
{
	GLuint gl_program = GProg[prog_id].GetId();

	if(render_target) {
		glBindFramebuffer(GL_FRAMEBUFFER, render_target->GetFramebufferId());
		glViewport (0, 0, render_target->GetWidth(), render_target->GetHeight());
		glCheck();
	}

	glUseProgram(gl_program);
	glCheck();

	glUniform2f(glGetUniformLocation(gl_program, "offset"), x0, y0);
	glUniform2f(glGetUniformLocation(gl_program, "scale"), x1 - x0, y1 - y0);
	glUniform2f(glGetUniformLocation(gl_program, "texelsize"), 1.0f/texture->GetWidth(), 1.0f/texture->GetHeight());
	glUniform1i(glGetUniformLocation(gl_program, "tex"), 0);
	glCheck();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);
	glCheck();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->GetId());
	glCheck();

	GLuint loc = glGetAttribLocation(gl_program, "vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);
	glCheck();
	glEnableVertexAttribArray(loc);
	glCheck();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glCheck();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	if(render_target){
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, GScreenWidth, GScreenHeight);
	}
}

void DrawDisparityRect(GfxTexture* tex_left, GfxTexture* tex_right, GfxTexture* tex_dispmap, int d, float x0, float y0, float x1, float y1)
{
	GLuint gl_program = GProg[SHAD_DMAP].GetId();

	glBindFramebuffer(GL_FRAMEBUFFER, tex_dispmap->GetFramebufferId());
	glViewport (0, 0, tex_dispmap->GetWidth(), tex_dispmap->GetHeight());
	glCheck();

	glUseProgram(gl_program);
	glCheck();

	glUniform2f(glGetUniformLocation(gl_program, "offset"), x0, y0);
	glUniform2f(glGetUniformLocation(gl_program, "scale"), x1 - x0, y1 - y0);
	glUniform2f(glGetUniformLocation(gl_program, "texelsize"), 1.0f/tex_left->GetWidth(), 1.0f/tex_left->GetHeight());
	glUniform1i(glGetUniformLocation(gl_program, "tex_left"), 0);
	glUniform1i(glGetUniformLocation(gl_program, "tex_right"), 1);
	glUniform1i(glGetUniformLocation(gl_program, "tex_dispmap"), 2);
	glUniform1i(glGetUniformLocation(gl_program, "disp"), d);
	glCheck();

	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);
	glCheck();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_left->GetId());
	glCheck();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_right->GetId());
	glCheck();
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex_dispmap->GetId());
	glCheck();
	glActiveTexture(GL_TEXTURE0);

	GLuint loc = glGetAttribLocation(gl_program, "vertex");
	glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 16, 0);
	glCheck();
	glEnableVertexAttribArray(loc);
	glCheck();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glCheck();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, GScreenWidth, GScreenHeight);
}

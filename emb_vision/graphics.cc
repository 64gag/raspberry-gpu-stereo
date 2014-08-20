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
#include "emb_vision.h"

uint32_t GScreenWidth;
uint32_t GScreenHeight;
EGLDisplay GDisplay;
EGLSurface GSurface;
EGLContext GContext;

GLuint GQuadVertexBuffer;

void InitGraphics(std::vector<GLTexture> &textures, std::vector<GLShader> &shaders, std::vector<GLProgram> &programs)
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

	/* Get an EGL display connection */
	GDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(GDisplay!=EGL_NO_DISPLAY);

	/* Initialize the EGL display connection */
	result = eglInitialize(GDisplay, NULL, NULL);
	assert(EGL_FALSE != result);
	
	/* Get an appropriate EGL frame buffer configuration */
	result = eglChooseConfig(GDisplay, attribute_list, &config, 1, &num_config);
	assert(EGL_FALSE != result);
	
	/* Get an appropriate EGL frame buffer configuration */
	result = eglBindAPI(EGL_OPENGL_ES_API);
	assert(EGL_FALSE != result);
	
	/* Create an EGL rendering context */
	GContext = eglCreateContext(GDisplay, config, EGL_NO_CONTEXT, context_attributes);
	assert(GContext!=EGL_NO_CONTEXT);
	
	/* Create an EGL window surface */
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

	GSurface = eglCreateWindowSurface( GDisplay, config, &nativewindow, NULL );
	assert(GSurface != EGL_NO_SURFACE);

	/* Connect the context to the surface */
	result = eglMakeCurrent(GDisplay, GSurface, GSurface, GContext);
	assert(EGL_FALSE != result);

	/* Set background color and clear buffers */
	glClearColor(0.15f, 0.25f, 0.35f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT );

	textures.push_back(GLTexture(IMG_W, IMG_H, GL_LUMINANCE));
	textures.push_back(GLTexture(IMG_W, IMG_H, GL_LUMINANCE));
	textures.push_back(GLTexture(256, 256, GL_RGBA));
	textures.push_back(GLTexture(16, 16, GL_RGBA));
	textures.push_back(GLTexture(32, 32, GL_RGBA));
	textures.push_back(GLTexture(64, 64, GL_RGBA));
	textures.push_back(GLTexture(128, 128, GL_RGBA));
	textures.push_back(GLTexture(256, 256, GL_RGBA));
	textures.push_back(GLTexture(16, 16, GL_RGBA));
	textures.push_back(GLTexture(16, 2, GL_RGBA));
	textures.push_back(GLTexture(2, 2, GL_RGBA));

	for(std::vector<GLTexture>::size_type i = 2; i < textures.size(); i++) {
		textures[i].GenerateFramebuffer();
	}

	shaders.push_back(GLShader("simple", GL_VERTEX_SHADER));
	shaders.push_back(GLShader("crop", GL_VERTEX_SHADER));
	shaders.push_back(GLShader("interpolate_xy", GL_VERTEX_SHADER));
	shaders.push_back(GLShader("flip", GL_VERTEX_SHADER));
	shaders.push_back(GLShader("left", GL_VERTEX_SHADER));
	shaders.push_back(GLShader("diagonals", GL_VERTEX_SHADER));
	shaders.push_back(GLShader("line8", GL_VERTEX_SHADER));

	shaders.push_back(GLShader("simple", GL_FRAGMENT_SHADER));
	shaders.push_back(GLShader("channelpack", GL_FRAGMENT_SHADER));
	shaders.push_back(GLShader("dmap0", GL_FRAGMENT_SHADER));
	shaders.push_back(GLShader("dinterpolate", GL_FRAGMENT_SHADER));
	shaders.push_back(GLShader("dmapN", GL_FRAGMENT_SHADER));
	shaders.push_back(GLShader("contrast", GL_FRAGMENT_SHADER));
	shaders.push_back(GLShader("dilate", GL_FRAGMENT_SHADER));
	shaders.push_back(GLShader("erode", GL_FRAGMENT_SHADER));

	programs.push_back(GLProgram(&shaders[VS_SIMPLE], &shaders[FS_SIMPLE + VS_COUNT]));
	programs.push_back(GLProgram(&shaders[VS_CROP], &shaders[FS_PACK + VS_COUNT]));
	programs.push_back(GLProgram(&shaders[VS_LEFT], &shaders[FS_DMAP0 + VS_COUNT]));
	programs.push_back(GLProgram(&shaders[VS_INTERPOLATE], &shaders[FS_DINTERPOLATE + VS_COUNT]));
	programs.push_back(GLProgram(&shaders[VS_SIMPLE], &shaders[FS_DMAPN + VS_COUNT]));
	programs.push_back(GLProgram(&shaders[VS_SIMPLE], &shaders[FS_CONTRAST + VS_COUNT]));
	programs.push_back(GLProgram(&shaders[VS_DIAGS], &shaders[FS_DILATE + VS_COUNT]));
	programs.push_back(GLProgram(&shaders[VS_LINE8], &shaders[FS_ERODE + VS_COUNT]));
	programs.push_back(GLProgram(&shaders[VS_FLIP], &shaders[FS_SIMPLE + VS_COUNT]));

	/* Create an ickle vertex buffer */
	static const GLfloat quad_vertex_positions[] = {
		0.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};

	glGenBuffers(1, &GQuadVertexBuffer);	
	glBindBuffer(GL_ARRAY_BUFFER, GQuadVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertex_positions), quad_vertex_positions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

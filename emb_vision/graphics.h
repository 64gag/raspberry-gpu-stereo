#include <vector>
#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "gltexture.h"
#include "glshader.h"
#include "glprogram.h"

enum VertexShadersT {
	VS_SIMPLE = 0,
	VS_CROP,
	VS_INTERPOLATE,
	VS_FLIP,
	VS_LEFT,
	VS_DIAGS,
	VS_LINE8,
	VS_COUNT
};

enum FragmentShadersT {
	FS_SIMPLE = 0,
	FS_PACK,
	FS_DMAP0,
	FS_DINTERPOLATE,
	FS_DMAPN,
	FS_CONTRAST,
	FS_DILATE,
	FS_ERODE,
	FS_COUNT
};

enum TexturesT {
	TEX_LEFT = 0,
	TEX_RIGHT,
	TEX_STEREO,
	TEX_DMAP16,
	TEX_DMAP32,
	TEX_DMAP64,
	TEX_DMAP128,
	TEX_DMAP256,
	TEX_FILTER,
	TEX_MAX_HOR,
	TEX_MAX_VER,
	TEX_COUNT
};

void InitGraphics(std::vector<GLTexture> &textures, std::vector<GLShader> &shaders, std::vector<GLProgram> &programs);

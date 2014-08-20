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
	TEX_INLEFT = 0,
	TEX_INRIGHT,
	TEX_ROI,
	TEX_DMAPL4,
	TEX_DMAPL3,
	TEX_DMAPL2,
	TEX_DMAPL1,
	TEX_DMAPL0,
	TEX_FILTER,
	TEX_MAXHOR,
	TEX_MAXVER,
	TEX_COUNT
};

void InitGraphics(std::vector<GLTexture> &textures, std::vector<GLShader> &shaders, std::vector<GLProgram> &programs);

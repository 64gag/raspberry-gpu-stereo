#ifndef GLTEXTURE_H_
#define GLTEXTURE_H_

class GLTexture
{
	int width_;
	int height_;
	GLuint id_;
	GLint format_;
	GLuint framebuffer_id_;
public:
	GLTexture() : width_(0), height_(0), id_(0), format_(0), framebuffer_id_(0) {}
	GLTexture(int width, int height, GLint format, const void* data = 0);
	~GLTexture() {}
	
	bool GenerateFramebuffer();
	void SetPixels(const void* data);
	GLuint id() { return id_; }
	GLuint framebuffer_id() { return framebuffer_id_; }
	int width() {return width_;}
	int height() {return height_;}
};

#endif /* GLTEXTURE_H_ */

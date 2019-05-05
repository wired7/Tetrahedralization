#pragma once
#include "Decorator.h"
#include "glew.h"

class DecoratedFrameBuffer : public Decorator<DecoratedFrameBuffer>
{
protected:
	virtual void bindFBO(void);
	virtual void bindTexture(void) = 0;
	virtual void bindRBO(void);
public:
	GLuint FBO = 0;
	GLuint texture;
	GLuint RBO;
	GLenum type;
	int attachmentNumber = 0;
	int width;
	int height;
	DecoratedFrameBuffer() {};
	DecoratedFrameBuffer(int width, int height, std::string signature, GLenum type);
	DecoratedFrameBuffer(DecoratedFrameBuffer* child, int width, int height, std::string signature, GLenum type);
	~DecoratedFrameBuffer() {};

	void drawBuffers(void);
	void drawBuffer(std::string signature);
	void drawBuffers(std::vector<std::string> signatures);

	int bindTexturesForPass(int textureOffset = 0);
	void bindTexturesForPass(std::string signature);
	void bindTexturesForPass(std::vector<std::string> signatures);

	DecoratedFrameBuffer* make(void) { return NULL; };
	std::string printOwnProperties(void);
};

// TODO : make singleton pattern
class DefaultFrameBuffer : public DecoratedFrameBuffer
{
protected:
	void bindFBO(void);
	void bindTexture(void);
	void bindRBO(void);
	int bindTexturesForPass(int textureOffset = 0);
public:
	DefaultFrameBuffer();
	~DefaultFrameBuffer() {};
};

class ImageFrameBuffer : public DecoratedFrameBuffer
{
protected:
	virtual void bindTexture(void);
public:
	ImageFrameBuffer(int width, int height, std::string signature);
	ImageFrameBuffer(DecoratedFrameBuffer* child, int width, int height, std::string signature);
	~ImageFrameBuffer() {};
};

class MSImageFrameBuffer : public DecoratedFrameBuffer
{
protected:
	virtual void bindTexture(void);
public:
	MSImageFrameBuffer(int width, int height, std::string signature);
	MSImageFrameBuffer(DecoratedFrameBuffer* child, int width, int height, std::string signature);
	~MSImageFrameBuffer() {};
};

class PickingBuffer : public DecoratedFrameBuffer
{
protected:
	virtual void bindTexture(void);
public:
	PickingBuffer(int width, int height, std::string signature);
	PickingBuffer(DecoratedFrameBuffer* child, int width, int height, std::string signature);
	~PickingBuffer() {};

	GLuint* getValues(int x, int y, int sampleW = 1, int sampleH = 1);
};
#pragma once
#include "FrameBuffer.h"

DecoratedFrameBuffer::DecoratedFrameBuffer(int width, int height, std::string signature, GLenum type) :
	Decorator<DecoratedFrameBuffer>(nullptr, signature), width(width), height(height), type(type)
{
}

DecoratedFrameBuffer::DecoratedFrameBuffer(DecoratedFrameBuffer* child, int width, int height, std::string signature, GLenum type) :
	Decorator<DecoratedFrameBuffer>(child, signature), width(width), height(height), type(type)
{
	FBO = child->FBO;
	attachmentNumber = child->attachmentNumber + 1;
}

void DecoratedFrameBuffer::bindFBO()
{
	if (FBO == 0)
	{
		glGenFramebuffers(1, &FBO);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void DecoratedFrameBuffer::bindTexture()
{

}

void DecoratedFrameBuffer::bindRBO()
{
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
}

void DecoratedFrameBuffer::drawBuffers(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	std::vector<GLenum> buff;
	DecoratedFrameBuffer* currentBuffer = this;
	do
	{
		buff.insert(buff.begin(), GL_COLOR_ATTACHMENT0 + currentBuffer->attachmentNumber);
		currentBuffer = currentBuffer->child;
	} while (currentBuffer != nullptr);

	glDrawBuffers(buff.size(), &(buff[0]));
}

int DecoratedFrameBuffer::bindTexturesForPass(int textureOffset)
{
	DecoratedFrameBuffer* currentBuffer = this;
	int count = textureOffset;
	for (; currentBuffer != nullptr; count++)
	{
		currentBuffer = currentBuffer->child;
	}

	currentBuffer = this;
	for (int i = count - 1; i >= textureOffset; i--)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(currentBuffer->type, currentBuffer->texture);
		currentBuffer = currentBuffer->child;
	}

	return count;
}

std::string DecoratedFrameBuffer::printOwnProperties(void)
{
	return std::to_string(attachmentNumber) + "\n";
}

DefaultFrameBuffer::DefaultFrameBuffer()
{
	FBO = 0;
	signature = "DEFAULT";
}

void DefaultFrameBuffer::bindFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void DefaultFrameBuffer::bindTexture()
{

}

void DefaultFrameBuffer::bindRBO()
{

}

int DefaultFrameBuffer::bindTexturesForPass(int textureOffset)
{
	return 0;
}

ImageFrameBuffer::ImageFrameBuffer(int width, int height, std::string signature) : DecoratedFrameBuffer(width, height, signature, GL_TEXTURE_2D)
{
	bindFBO();
	bindTexture();
	bindRBO();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ImageFrameBuffer::ImageFrameBuffer(DecoratedFrameBuffer* child, int width, int height, std::string signature) :
	DecoratedFrameBuffer(child, width, height, signature, GL_TEXTURE_2D)
{
	bindFBO();
	bindTexture();
	bindRBO();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ImageFrameBuffer::bindTexture()
{
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(type, texture);

	glTexImage2D(type, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(type, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentNumber, type, texture, 0);
}

MSImageFrameBuffer::MSImageFrameBuffer(int width, int height, std::string signature) : DecoratedFrameBuffer(width, height, signature, GL_TEXTURE_2D_MULTISAMPLE)
{
	bindFBO();
	bindTexture();
	bindRBO();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

MSImageFrameBuffer::MSImageFrameBuffer(DecoratedFrameBuffer* child, int width, int height, std::string signature) :
	DecoratedFrameBuffer(child, width, height, signature, GL_TEXTURE_2D_MULTISAMPLE)
{
	bindFBO();
	bindTexture();
	bindRBO();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MSImageFrameBuffer::bindTexture()
{
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(type, texture);

	glTexImage2DMultisample(type, 4, GL_RGB, width, height, GL_TRUE);
	glBindTexture(type, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentNumber, type, texture, 0);
}

PickingBuffer::PickingBuffer(DecoratedFrameBuffer* child, int width, int height, std::string signature) :
	DecoratedFrameBuffer(child, width, height, signature, GL_TEXTURE_2D)
{
	bindFBO();
	bindTexture();
	bindRBO();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

PickingBuffer::PickingBuffer(int width, int height, std::string signature) : DecoratedFrameBuffer(width, height, signature, GL_TEXTURE_2D)
{
	bindFBO();
	bindTexture();
	bindRBO();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PickingBuffer::bindTexture()
{
	glGenTextures(1, &texture);
	glBindTexture(type, texture);

	glTexImage2D(type, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	auto error = glGetError();

	if (error != GL_NO_ERROR)
		std::cout << error << std::endl;

	glBindTexture(type, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentNumber, type, texture, 0);
}

GLuint* PickingBuffer::getValues(int x, int y, int sampleW, int sampleH)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
	glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentNumber);
	int length = 4 * width * height;

	GLuint* data = new GLuint[4];

	glReadPixels(x, y, sampleW, sampleH, GL_RGBA_INTEGER, GL_UNSIGNED_INT, data);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	return data;
}
#pragma once
#include <map>
#include <string>
#include "glew.h"

struct Texture {
	GLuint id;
	GLfloat shininess = 45.0f;
	std::string type;
	std::string path;
};

class TextureManager
{
public:
	virtual ~TextureManager();
	static TextureManager* instance();

	GLint addTexture(std::string);
	GLint addTexture(const char*);

private:
	TextureManager();
	static TextureManager* self;
	std::map<std::string, Texture> textures;
};

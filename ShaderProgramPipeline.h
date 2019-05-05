#pragma once
#include <vector>
#include <string>
#include <glew.h>

class ShaderProgram;

class ShaderProgramPipeline
{
private:
	ShaderProgramPipeline(std::string s, bool alphaRendered = false);
	~ShaderProgramPipeline();
public:
	bool alphaRendered;
	static std::vector<ShaderProgramPipeline*> pipelines;
	static ShaderProgramPipeline* getPipeline(std::string s);
	std::string signature;
	GLuint pipeline;
	std::vector<ShaderProgram*> attachedPrograms;
	void attachProgram(ShaderProgram* program);
	void use(void);
	ShaderProgram* getProgramBySignature(std::string s);
	ShaderProgram* getProgramByEnum(GLenum e);
	GLint getUniformByID(std::string s);
};


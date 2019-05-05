#pragma once
#include <vector>
#include <string>
#include <tuple>
#include "glew.h"

class ShaderProgramPipeline;

enum UniformType {ONEUI, TWOUI, MATRIX4FV, VECTOR4FV, TEXTURE};

class ShaderProgram
{
protected:
	static ShaderProgram* getCompiledProgram(std::string filePath);
	static ShaderProgram* getCompiledProgramBySignature(std::string signature);
	ShaderProgram(std::string filePath, std::vector<std::tuple<const GLchar*, UniformType>> uIDs, std::string signature, GLenum shader, GLenum shaderBit);
	~ShaderProgram();
public:
	static std::vector<ShaderProgram*> compiledPrograms;
	template<class T> static T* getShaderProgram(std::string filePath, std::vector<std::tuple<const GLchar*, UniformType>> uIDs, std::string signature);
	template<class T> static T* getShaderProgram(std::string filePath);
	std::string filePath;
	std::string programString;
	GLuint program;
	GLenum shader;
	GLenum shaderBit;
	std::string signature;
	std::vector<std::tuple<std::string, GLint, UniformType>> uniformIDs;
	virtual void bindShaderProgram();
	virtual void loadShaderProgram();
	virtual void attachToPipeline(ShaderProgramPipeline* pipeline);
	GLint getLocationBySignature(std::string s);
};

template<class T> T* ShaderProgram::getShaderProgram(std::string filePath, std::vector<std::tuple<const GLchar*, UniformType>> uIDs, std::string signature)
{
	T* prog = (T*)getCompiledProgram(filePath);

	if (prog == nullptr)
	{
		prog = new T(filePath, uIDs, signature);

		if (getCompiledProgram(filePath) == nullptr)
		{
			delete prog;
			prog = nullptr;
		}
	}

	return prog;
}

template<class T> T* ShaderProgram::getShaderProgram(std::string signature)
{
	return (T*)getCompiledProgramBySignature(signature);
}

class VertexShaderProgram : public ShaderProgram
{
public:
	VertexShaderProgram(std::string filePath, std::vector<std::tuple<const GLchar*, UniformType>> uIDs, std::string signature);
	~VertexShaderProgram() {};
};

class FragmentShaderProgram : public ShaderProgram
{
public:
	FragmentShaderProgram(std::string filePath, std::vector<std::tuple<const GLchar*, UniformType>> uIDs, std::string signature);
	~FragmentShaderProgram() {};
};

class GeometryShaderProgram : public ShaderProgram
{
public:
	GeometryShaderProgram(std::string filePath, std::vector<std::tuple<const GLchar*, UniformType>> uIDs, std::string signature);
	~GeometryShaderProgram() {};
	void setGeometryInputType(GLuint type);
	void setGeometryOutputType(GLuint type);
	void setGeometryOutputVertexCount(GLuint count);
};
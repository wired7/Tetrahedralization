#pragma once
#include "ShaderProgram.h"
#include "ShaderProgramPipeline.h"
#include <iostream>
#include <fstream>
#include <sstream>

std::vector<ShaderProgram*> ShaderProgram::compiledPrograms;

ShaderProgram* ShaderProgram::getCompiledProgram(std::string filePath)
{
	for (int i = 0; i < compiledPrograms.size(); i++)
	{
		if (compiledPrograms[i]->filePath == filePath)
		{
			return compiledPrograms[i];
		}
	}

	return nullptr;
}

ShaderProgram* ShaderProgram::getCompiledProgramBySignature(std::string signature)
{
	for (int i = 0; i < compiledPrograms.size(); i++)
	{
		if (compiledPrograms[i]->signature == signature)
		{
			return compiledPrograms[i];
		}
	}

	return nullptr;
}


ShaderProgram::ShaderProgram(std::string filePath, std::vector<std::tuple<const GLchar*, UniformType>> uIDs, std::string signature, GLenum shader, GLenum shaderBit) : 
	filePath(filePath), signature(signature), shader(shader), shaderBit(shaderBit)
{
	loadShaderProgram();
	bindShaderProgram();

	for (int i = 0; i < uIDs.size(); i++)
	{
		uniformIDs.push_back(std::tuple<std::string, GLint, UniformType>(std::get<0>(uIDs[i]), glGetUniformLocation(program,
							std::get<0>(uIDs[i])), std::get<1>(uIDs[i])));
	}
}

ShaderProgram::~ShaderProgram()
{
	glGetError();
	std::cout << "DELETING SHADER PROGRAM " << signature << std::endl;
	glDeleteProgram(program);
	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		std::cout << "ERROR DELETING PROGRAM" << std::endl;
	}
}

void ShaderProgram::loadShaderProgram()
{
	std::string shaderCodeString;
	std::ifstream shaderCodeStream(filePath, std::ios::in);
	if (shaderCodeStream.is_open()) {
		std::string Line = "";
		while (getline(shaderCodeStream, Line))
			shaderCodeString += "\n" + Line;
		shaderCodeStream.close();
		programString = shaderCodeString;
	}
	else {
		std::cout << "Impossible to open" << filePath << "Are you in the right directory?" << std::endl;
	}
}

void ShaderProgram::bindShaderProgram(void)
{
	std::cout << "BINDING " << signature << " SHADER..." << std::endl;

	const char* pStringPointer = programString.c_str();

	program = glCreateShaderProgramv(shader, 1, &pStringPointer);

	// Check shader program
	GLint Result = GL_FALSE;
	int InfoLogLength;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &Result);

	std::cout << Result << " ACTIVE UNIFORMS" << std::endl;

	glGetProgramiv(program, GL_ATTACHED_SHADERS, &Result);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> errorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(program, InfoLogLength, NULL, &errorMessage[0]);
		printf("%s\n", &errorMessage[0]);
	}
	else
	{
		compiledPrograms.push_back(this);
	}

	std::cout << std::endl;
}

void ShaderProgram::attachToPipeline(ShaderProgramPipeline* pipeline)
{
	pipeline->attachProgram(this);
}

GLint ShaderProgram::getLocationBySignature(std::string s)
{
	for (int i = 0; i < uniformIDs.size(); i++)
	{
		if (std::get<0>(uniformIDs[i]) == s)
		{
			return std::get<1>(uniformIDs[i]);
		}
	}

	return 0;
}

VertexShaderProgram::VertexShaderProgram(std::string filePath, std::vector<std::tuple<const GLchar*, UniformType>> uIDs, std::string signature) :
	ShaderProgram(filePath, uIDs, signature, GL_VERTEX_SHADER, GL_VERTEX_SHADER_BIT)
{
}

FragmentShaderProgram::FragmentShaderProgram(std::string filePath, std::vector<std::tuple<const GLchar*, UniformType>> uIDs, std::string signature) :
	ShaderProgram(filePath, uIDs, signature, GL_FRAGMENT_SHADER, GL_FRAGMENT_SHADER_BIT)
{
}

GeometryShaderProgram::GeometryShaderProgram(std::string filePath, std::vector<std::tuple<const GLchar*, UniformType>> uIDs, std::string signature) :
	ShaderProgram(filePath, uIDs, signature, GL_GEOMETRY_SHADER, GL_GEOMETRY_SHADER_BIT)
{

}

void GeometryShaderProgram::setGeometryInputType(GLuint type)
{

}

void GeometryShaderProgram::setGeometryOutputType(GLuint type)
{

}

void GeometryShaderProgram::setGeometryOutputVertexCount(GLuint count)
{

}
#pragma once
#include "ShaderProgramPipeline.h"
#include "ShaderProgram.h"
#include <iostream>

std::vector<ShaderProgramPipeline*> ShaderProgramPipeline::pipelines;

ShaderProgramPipeline* ShaderProgramPipeline::getPipeline(std::string s)
{
	for (int i = 0; i < pipelines.size(); i++)
	{
		if (pipelines[i]->signature == s)
		{
			return pipelines[i];
		}
	}

	auto p = new ShaderProgramPipeline(s);

	for (int i = 0; i < pipelines.size(); i++)
	{
		if (pipelines[i]->signature == s)
		{
			return pipelines[i];
		}
	}

	delete p;
	return nullptr;
}

ShaderProgramPipeline::ShaderProgramPipeline(std::string s, bool alphaRendered) : signature(s), alphaRendered(alphaRendered)
{
	glGetError();

	std::cout << "CREATING PROGRAM PIPELINE..." << std::endl;
	glGenProgramPipelines(1, &pipeline);
	glBindProgramPipeline(pipeline);
	
	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		std::cout << "ERROR CREATING PROGRAM PIPELINE!" << std::endl;
	}
	else
	{
		pipelines.push_back(this);
	}

	std::cout << std::endl;
}

ShaderProgramPipeline::~ShaderProgramPipeline()
{
	glGetError();
	std::cout << "DELETING SHADER PROGRAM " << signature << std::endl;
	glDeleteProgramPipelines(1, &pipeline);
	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		std::cout << "ERROR DELETING PROGRAM" << std::endl;
	}
}

void ShaderProgramPipeline::attachProgram(ShaderProgram* program)
{
	glGetError();

	std::cout << "ATTACHING " << program->signature << " TO PIPELINE " << pipeline << std::endl;

	glUseProgramStages(pipeline, program->shaderBit, program->program);
	
	// Check program pipeline
	GLint Result = GL_FALSE;
	glGetProgramPipelineiv(pipeline, program->shader, &Result);
	
	glGetProgramiv(program->program, GL_ACTIVE_UNIFORMS, &Result);

	std::cout << (int)Result << " ACTIVE UNIFORMS" << std::endl;

	GLenum error = glGetError();

	if (error == GL_NO_ERROR && Result == program->program)
	{
		std::cout << "SHADER ATTACHED SUCCESSFULY!" << std::endl;
	}
	else if (error != GL_NO_ERROR)
	{
		std::cout << "AN ERROR HAS OCCURRED" << std::endl;
	}

	std::cout << std::endl;

	glGetError();


	attachedPrograms.push_back(program);
}

void ShaderProgramPipeline::use(void)
{
	glBindProgramPipeline(pipeline);
}

ShaderProgram* ShaderProgramPipeline::getProgramBySignature(std::string signature)
{
	for (int i = 0; i < attachedPrograms.size(); i++)
	{
		if (attachedPrograms[i]->signature == signature)
		{
			return attachedPrograms[i];
		}
	}

	return nullptr;
}

ShaderProgram* ShaderProgramPipeline::getProgramByEnum(GLenum e)
{
	for (int i = 0; i < attachedPrograms.size(); i++)
	{
		if (attachedPrograms[i]->shader == e)
		{
			return attachedPrograms[i];
		}
	}

	return nullptr;
}

GLint ShaderProgramPipeline::getUniformByID(std::string signature)
{
	for (int i = 0; i < attachedPrograms.size(); i++)
	{
		for (const auto& uniformID : attachedPrograms[i]->uniformIDs)
		{
			if (std::get<0>(uniformID.second) == signature)
			{
				return std::get<1>(uniformID.second);
			}
		}
	}

	return -1;
}
#pragma once
#include "Pass.h"
#include "WindowContext.h"
#include "GeometricalMeshObjects.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "ShaderProgramPipeline.h"

Pass::Pass()
{
}

Pass::Pass(std::map<std::string, ShaderProgramPipeline*> shaderPipelines, std::vector<Pass*> neighbors, std::string signature) :
	DirectedGraphNode(neighbors, signature), shaderPipelines(shaderPipelines)
{
	registerUniforms();
}

Pass::Pass(std::map<std::string, ShaderProgramPipeline*> shaderPipelines, std::string signature) : DirectedGraphNode(signature), shaderPipelines(shaderPipelines)
{
	registerUniforms();
}

Pass::~Pass()
{
}

void Pass::execute(void)
{
	currentCount++;
	if (currentCount < incomingCount)
	{
		return;
	}

	currentCount = 0;

	executeOwnBehaviour();

	for (int i = 0; i < neighbors.size(); i++)
	{
		neighbors[i]->execute();
	}
}

void Pass::registerUniforms(void)
{
	for (const auto& pipeline : shaderPipelines)
	{
		uniformIDs[pipeline.second->signature] = std::vector<std::tuple<std::string, GLuint, GLuint, UniformType>>();

		for (const auto& program : pipeline.second->attachedPrograms)
		{
			for (auto& uniformID : program->uniformIDs)
			{
				uniformIDs[pipeline.second->signature].push_back(std::tuple<std::string, GLuint, GLuint, UniformType>(std::get<0>(uniformID),
																											program->program,
																											std::get<1>(uniformID),
																											std::get<2>(uniformID)));
			}
		}
	}
}

int Pass::getUniformIndexBySignature(const std::string& programSignature, std::string signature)
{
	for (int j = 0; j < uniformIDs[programSignature].size(); j++)
	{
		if (std::get<0>(uniformIDs[programSignature][j]) == signature)
		{
			return j;
		}
	}

	return -1;
}

void Pass::addNeighbor(Pass* neighbor)
{
	DirectedGraphNode::addNeighbor(neighbor);
	neighbor->incomingCount++;
}

RenderPass::RenderPass(std::map<std::string, ShaderProgramPipeline*> shaderPipelines, std::string signature, DecoratedFrameBuffer* frameBuffer, bool terminal) :
	Pass(shaderPipelines, signature), frameBuffer(frameBuffer), terminal(terminal)
{
	for (const auto& pipeline : shaderPipelines)
	{
		renderableObjects[pipeline.second->signature] = std::vector<Graphics::DecoratedGraphicsObject*>();
		floatTypeUniformPointers[pipeline.second->signature] = std::vector<std::tuple<int, GLfloat*>>();
		uintTypeUniformPointers[pipeline.second->signature] = std::vector<std::tuple<int, GLuint*>>();
		intTypeUniformPointers[pipeline.second->signature] = std::vector<std::tuple<int, GLint*>>();
		uintTypeUniformValues[pipeline.second->signature] = std::vector<std::tuple<int, GLuint>>();
	}
}

RenderPass::~RenderPass()
{

}

void RenderPass::initFrameBuffers(void)
{
}

void RenderPass::clearRenderableObjects(const std::string& signature)
{
	renderableObjects[signature].clear();
}

void RenderPass::setRenderableObjects(std::map<std::string, std::vector<Graphics::DecoratedGraphicsObject*>> input)
{
	renderableObjects = input;
}

void RenderPass::addRenderableObjects(Graphics::DecoratedGraphicsObject* input, const std::string& programSignature)
{
	renderableObjects[programSignature].push_back(input);
}

void RenderPass::setProbe(const std::string& passSignature, const std::string& fbSignature)
{
}

void RenderPass::setFrameBuffer(DecoratedFrameBuffer* fb)
{
	frameBuffer = fb;
}

void RenderPass::setTextureUniforms(const std::string& programSignature)
{
	for (int i = 0, count = 0; i < uniformIDs[programSignature].size(); i++)
	{
		if (std::get<3>(uniformIDs[programSignature][i]) == TEXTURE)
		{
			glProgramUniform1iv(std::get<1>(uniformIDs[programSignature][i]), std::get<2>(uniformIDs[programSignature][i]), 1, &count);
			count++;
		}
	}

	// IMPLEMENT FOR OTHER DATA TYPES
	for (int i = 0; i < floatTypeUniformPointers[programSignature].size(); i++)
	{
		auto uID = uniformIDs[programSignature][std::get<0>(floatTypeUniformPointers[programSignature][i])];

		if (std::get<3>(uID) == MATRIX4FV)
		{
			glProgramUniformMatrix4fv(std::get<1>(uID), std::get<2>(uID), 1, GL_FALSE, std::get<1>(floatTypeUniformPointers[programSignature][i]));
		}
		else if (std::get<3>(uID) == VECTOR4FV)
		{
			glProgramUniform4fv(std::get<1>(uID), std::get<2>(uID), 1, std::get<1>(floatTypeUniformPointers[programSignature][i]));
		}
	}

	for (int i = 0; i < uintTypeUniformValues[programSignature].size(); i++)
	{
		auto uID = uniformIDs[programSignature][std::get<0>(uintTypeUniformValues[programSignature][i])];

		if (std::get<3>(uID) == ONEUI)
		{
			glProgramUniform1ui(std::get<1>(uID), std::get<2>(uID), std::get<1>(uintTypeUniformValues[programSignature][i]));
		}
	}
}

void RenderPass::clearBuffers(void)
{
}

void RenderPass::configureGL(const std::string& programSignature)
{
}

void RenderPass::renderObjects(const std::string& programSignature)
{
	for (int i = 0; i < renderableObjects[programSignature].size(); i++)
	{
		setupObjectwiseUniforms(programSignature, i);
		renderableObjects[programSignature][i]->enableBuffers();
		renderableObjects[programSignature][i]->draw();
	}
}

void RenderPass::setupObjectwiseUniforms(const std::string& programSignature, int index)
{
}

void RenderPass::executeOwnBehaviour()
{
	// Set input textures from incoming passes for this stage
	for (int i = 0, count = 0; i < parents.size(); i++)
	{
		auto parent = dynamic_cast<RenderPass*>(parents[i]);

		if (parent == nullptr)
		{
			continue;
		}

		count += parent->frameBuffer->bindTexturesForPass(count);
	}

	// Set output textures
	frameBuffer->drawBuffers();

	// Clear buffers
	if (clearBuff)
	{
		clearBuffers();
	}

	for (const auto pipeline : shaderPipelines)
	{
		// GL configuration
		configureGL(pipeline.second->signature);

		// Shader setup
		pipeline.second->use();

		// Texture uniforms setup
		setTextureUniforms(pipeline.second->signature);

		// Object rendering
		renderObjects(pipeline.second->signature);
	}
}

GeometryPass::GeometryPass(std::map<std::string, ShaderProgramPipeline*> shaderPipelines,
			  std::string signature,
			  DecoratedFrameBuffer* frameBuffer,
			  int pickingBufferCount,
			  int stencilBufferCount,
			  bool terminal) :
	pickingBufferCount(pickingBufferCount), stencilBufferCount(stencilBufferCount), RenderPass(shaderPipelines, signature, frameBuffer, terminal)
{
	initFrameBuffers();
}

void GeometryPass::initFrameBuffers(void)
{
	auto widthHeight = WindowContext::context->getSize();

	int width = widthHeight.first;
	int height = widthHeight.second;

	auto colorsBuffer = new ImageFrameBuffer(width, height, "COLORS");
	auto normalsBuffer = new ImageFrameBuffer(colorsBuffer, width, height, "NORMALS");
	DecoratedFrameBuffer* lastBuffer = new ImageFrameBuffer(normalsBuffer, width, height, "POSITIONS");

	for (int i = 0; i < pickingBufferCount; ++i)
	{
		lastBuffer = new PickingBuffer(lastBuffer, width, height, "PICKING" + std::to_string(i));
	}

	for (int i = 0; i < stencilBufferCount; ++i)
	{
		lastBuffer = new PickingBuffer(lastBuffer, width, height, "STENCIL" + std::to_string(i));
	}

	frameBuffer = lastBuffer;
}


void GeometryPass::clearBuffers(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GeometryPass::configureGL(const std::string& programSignature)
{
	if (!shaderPipelines[programSignature]->alphaRendered)
	{
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glDisable(GL_CULL_FACE);
	glDisable(GL_MULTISAMPLE);

//	glEnable(GL_CULL_FACE);
}

void GeometryPass::setupObjectwiseUniforms(const std::string& programSignature, int index)
{
	GLuint p = shaderPipelines[programSignature]->getProgramByEnum(GL_VERTEX_SHADER)->program;
	glProgramUniformMatrix4fv(p, glGetUniformLocation(p, "Model"), 1, GL_FALSE, &(renderableObjects[programSignature][index]->getModelMatrix()[0][0]));
}

void GeometryPass::setupCamera(Camera* cam)
{
	for (const auto pipeline : shaderPipelines)
	{
		updatePointerBySignature<float>(pipeline.second->signature, "Projection", &(cam->Projection[0][0]));
		updatePointerBySignature<float>(pipeline.second->signature, "View", &(cam->View[0][0]));
	}
}

void GeometryPass::setupVec4f(glm::vec4& input, std::string name)
{
	for (const auto pipeline : shaderPipelines)
	{
		updatePointerBySignature<float>(pipeline.second->signature, name, &(input[0]));
	}
}

void GeometryPass::setupOnHover(unsigned int id)
{
	for (const auto pipeline : shaderPipelines)
	{
		updateValueBySignature<unsigned int>(pipeline.second->signature, "selectedRef", id);
	}
}

LightPass::LightPass(std::map<std::string, ShaderProgramPipeline*> shaderPipelines, bool terminal) : 
	RenderPass(shaderPipelines, "LIGHTPASS", nullptr)
{
	if (terminal)
	{
		frameBuffer = new DefaultFrameBuffer();
	}
	else
	{
		initFrameBuffers();
	}
}

void LightPass::initFrameBuffers(void)
{
	auto widthHeight = WindowContext::context->getSize();
	auto imageFB = new ImageFrameBuffer(widthHeight.first, widthHeight.second, "MAINIMAGE");

	frameBuffer = imageFB;
}

void LightPass::clearBuffers(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void LightPass::configureGL(const std::string& programSignature)
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
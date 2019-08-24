#pragma once
#include "Pass.h"
#include "WindowContext.h"
#include "GeometricalMeshObjects.h"
#include "FrameBuffer.h"
#include "ShaderProgramPipeline.h"

Pass::Pass()
{
}

Pass::Pass(std::map<std::string, ShaderProgramPipeline*> shaderPipelines, std::vector<Pass*> neighbors, std::string signature) :
	DirectedGraphNode(neighbors, signature), shaderPipelines(shaderPipelines)
{
}

Pass::Pass(std::map<std::string, ShaderProgramPipeline*> shaderPipelines, std::string signature) : DirectedGraphNode(signature), shaderPipelines(shaderPipelines)
{
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

void Pass::addNeighbor(Pass* neighbor)
{
	DirectedGraphNode::addNeighbor(neighbor);
	neighbor->incomingCount++;
}

RenderPass::RenderPass(std::map<std::string, ShaderProgramPipeline*> shaderPipelines, std::string signature, DecoratedFrameBuffer* frameBuffer, bool terminal) :
	Pass(shaderPipelines, signature), frameBuffer(frameBuffer), terminal(terminal)
{
	registerUniforms();

	for (const auto& pipeline : shaderPipelines)
	{
		renderableObjects[pipeline.second->signature] = std::map<std::string, Graphics::DecoratedGraphicsObject*>();
		floatTypeUniformPointers[pipeline.second->signature] = std::map<std::string, std::tuple<std::string, GLfloat*>>();
		uintTypeUniformPointers[pipeline.second->signature] = std::map<std::string, std::tuple<std::string, GLuint*>>();
		intTypeUniformPointers[pipeline.second->signature] = std::map<std::string, std::tuple<std::string, GLint*>>();
		uintTypeUniformValues[pipeline.second->signature] = std::map<std::string, std::tuple<std::string, GLuint>>();
	}
}

void RenderPass::clearRenderableObjects(const std::string& signature)
{
	renderableObjects[signature].clear();
}

void RenderPass::clearRenderableObjects(const std::string& signature, const std::string& programSignature)
{
	if (renderableObjects.find(programSignature) != renderableObjects.end())
	{
		renderableObjects[programSignature].erase(signature);

		if (renderableObjects[programSignature].empty())
		{
			renderableObjects.erase(programSignature);
		}
	}
}

void RenderPass::setRenderableObjects(std::map<std::string, std::map<std::string, Graphics::DecoratedGraphicsObject*>> input)
{
	renderableObjects = input;
}

void RenderPass::addRenderableObjects(Graphics::DecoratedGraphicsObject* input, const std::string& signature, const std::string& programSignature)
{
	renderableObjects[programSignature][signature] = input;
}

void RenderPass::setFrameBuffer(DecoratedFrameBuffer* fb)
{
	frameBuffer = fb;
}

void RenderPass::setTextureUniforms(const std::string& programSignature)
{
	auto idsByProgram = uniformIDs[programSignature];

	for (const auto& uniformID : idsByProgram)
	{
		if (std::get<3>(uniformID.second) == TEXTURE)
		{
			int pos = std::get<4>(uniformID.second);
			glProgramUniform1i(std::get<1>(uniformID.second), std::get<2>(uniformID.second), pos);
		}
	}

	auto floatPointersByProgram = floatTypeUniformPointers[programSignature];
	// IMPLEMENT FOR OTHER DATA TYPES
	for (const auto& ptr : floatPointersByProgram)
	{
		auto uID = uniformIDs[programSignature][std::get<0>(ptr.second)];

		if (std::get<3>(uID) == MATRIX4FV)
		{
			glProgramUniformMatrix4fv(std::get<1>(uID), std::get<2>(uID), 1, GL_FALSE, std::get<1>(ptr.second));
		}
		else if (std::get<3>(uID) == VECTOR4FV)
		{
			glProgramUniform4fv(std::get<1>(uID), std::get<2>(uID), 1, std::get<1>(ptr.second));
		}
	}

	auto intPointerByProgram = intTypeUniformPointers[programSignature];
	for (const auto& ptr : intPointerByProgram)
	{
		auto uID = uniformIDs[programSignature][std::get<0>(ptr.second)];
		
		if (std::get<3>(uID) == VECTOR2IV)
		{
			glProgramUniform2iv(std::get<1>(uID), std::get<2>(uID), 1, std::get<1>(ptr.second));
		}
	}

	auto uintValuesByProgram = uintTypeUniformValues[programSignature];
	for (const auto& val : uintValuesByProgram)
	{
		auto uID = uniformIDs[programSignature][std::get<0>(val.second)];

		if (std::get<3>(uID) == ONEUI)
		{
			glProgramUniform1ui(std::get<1>(uID), std::get<2>(uID), std::get<1>(val.second));
		}
	}
}

void RenderPass::setTextureToIgnore(const std::string& signature, const std::string& parentPassSignature)
{
	texturesToIgnore[parentPassSignature].insert(signature);
}

void RenderPass::renderObjects(const std::string& programSignature)
{
	auto objectsByProgram = renderableObjects[programSignature];
	for (const auto& object :objectsByProgram)
	{
		setupObjectwiseUniforms(programSignature, object.first);
		object.second->enableBuffers();
		object.second->draw();
	}
}

void RenderPass::registerUniforms(void)
{
	for (const auto& pipeline : shaderPipelines)
	{
		uniformIDs[pipeline.second->signature] = std::map<std::string, std::tuple<std::string, GLuint, GLuint, UniformType, int>>();

		for (const auto& program : pipeline.second->attachedPrograms)
		{
			for (auto& uniformIDPair : program->uniformIDs)
			{
				auto uniformID = uniformIDPair.second;

				uniformIDs[pipeline.second->signature][std::get<0>(uniformID)] = std::make_tuple(
					std::get<0>(uniformID),
					program->program,
					std::get<1>(uniformID),
					std::get<2>(uniformID),
					std::get<3>(uniformID));
			}
		}
	}
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

		std::unordered_set<std::string> ignoreSet;
		if (texturesToIgnore.find(parent->signature) != texturesToIgnore.end())
		{
			ignoreSet = texturesToIgnore[parent->signature];
		}

		count += parent->frameBuffer->bindTexturesForPass(ignoreSet, count);
	}

	// Set output textures
	frameBuffer->drawBuffers();

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

	auto colorsBuffer = new ImageFrameBuffer(width, height, "COLORS", glm::vec4(1.0f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	auto normalsBuffer = new ImageFrameBuffer(colorsBuffer, width, height, "NORMALS", glm::vec4(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DecoratedFrameBuffer* lastBuffer = new ImageFrameBuffer(normalsBuffer, width, height, "POSITIONS", glm::vec4(),
															GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

	glEnable(GL_CULL_FACE);
}

void GeometryPass::setupObjectwiseUniforms(const std::string& programSignature, const std::string& signature)
{
	GLuint p = shaderPipelines[programSignature]->getProgramByEnum(GL_VERTEX_SHADER)->program;
	glProgramUniformMatrix4fv(p, glGetUniformLocation(p, "Model"), 1, GL_FALSE, &(renderableObjects[programSignature][signature]->getModelMatrix()[0][0]));
}

void GeometryPass::setupCamera(Camera* cam)
{
	for (const auto pipeline : shaderPipelines)
	{
		updateFloatPointerBySignature<float>(pipeline.second->signature, "Projection", &(cam->Projection[0][0]));
		updateFloatPointerBySignature<float>(pipeline.second->signature, "View", &(cam->View[0][0]));
	}
}

void GeometryPass::setupVec4f(glm::vec4& input, std::string name)
{
	for (const auto pipeline : shaderPipelines)
	{
		updateFloatPointerBySignature<float>(pipeline.second->signature, name, &(input[0]));
	}
}

void GeometryPass::setupVec2i(glm::ivec2& input, std::string name)
{
	for (const auto pipeline : shaderPipelines)
	{
		updateIntPointerBySignature<int>(pipeline.second->signature, name, &(input[0]));
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
	auto imageFB = new ImageFrameBuffer(widthHeight.first, widthHeight.second, "MAINIMAGE", glm::vec4());

	frameBuffer = imageFB;
}

void LightPass::configureGL(const std::string& programSignature)
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
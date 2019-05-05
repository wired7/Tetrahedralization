#pragma once
#include "Camera.h"
#include "DirectedGraphNode.h"
#include "GraphicsObject.h"
#include <map>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <tuple>

class ShaderProgramPipeline;
class CLKernel;
class AbstractCLBuffer;
class DecoratedFrameBuffer;
enum UniformType;
class Pass : public DirectedGraphNode<Pass>
{
protected:
	int incomingCount = 0;
	int currentCount = 0;
	std::map<std::string, ShaderProgramPipeline*> shaderPipelines;
	virtual void executeOwnBehaviour() = 0;
public:
	std::map<std::string, std::vector<std::tuple<std::string, GLuint, GLuint, UniformType>>> uniformIDs;
	std::map<std::string, std::vector<std::tuple<int, GLfloat*>>> floatTypeUniformPointers;
	std::map<std::string, std::vector<std::tuple<int, GLuint*>>> uintTypeUniformPointers;
	std::map<std::string, std::vector<std::tuple<int, GLint*>>> intTypeUniformPointers;
	std::map<std::string, std::vector<std::tuple<int, GLuint>>> uintTypeUniformValues;
	Pass();
	Pass(std::map<std::string, ShaderProgramPipeline*> shaderPipelines, std::vector<Pass*> neighbors, std::string signature);
	Pass(std::map<std::string, ShaderProgramPipeline*> shaderPipelines, std::string signature);
	~Pass();
	virtual void execute(void);
	virtual void registerUniforms(void);
	virtual int getUniformIndexBySignature(const std::string& programSignature, std::string signature);
	template<typename T> void updatePointerBySignature(const std::string& programSignature, std::string signature, T* pointer);
	template<typename T> void updateValueBySignature(const std::string& programSignature, std::string signature, T value);
	virtual void addNeighbor(Pass* neighbor);
};

// IMPLEMENT FOR OTHER TYPES!!!
template<typename T> void Pass::updatePointerBySignature(const std::string& programSignature, std::string signature, T* pointer)
{
	if (std::is_same<T, GLfloat>::value || std::is_same<T, float>::value)
	{
		for (int i = 0; i < floatTypeUniformPointers[programSignature].size(); i++)
		{
			if (std::get<0>(uniformIDs[programSignature][std::get<0>(floatTypeUniformPointers[programSignature][i])]) == signature)
			{
				std::get<1>(floatTypeUniformPointers[programSignature][i]) = pointer;
				return;
			}
		}

		floatTypeUniformPointers[programSignature].push_back(std::tuple<int, T*>(getUniformIndexBySignature(programSignature, signature), pointer));
	}
}

template<typename T> void Pass::updateValueBySignature(const std::string& programSignature, std::string signature, T value)
{
	if (std::is_same<T, GLuint>::value || std::is_same<T, unsigned int>::value)
	{
		for (int i = 0; i < uintTypeUniformValues[programSignature].size(); i++)
		{
			if (std::get<0>(uniformIDs[programSignature][std::get<0>(uintTypeUniformValues[programSignature][i])]) == signature)
			{
				std::get<1>(uintTypeUniformValues[programSignature][i]) = value;
				return;
			}
		}

		uintTypeUniformValues[programSignature].push_back(std::tuple<int, T>(getUniformIndexBySignature(programSignature, signature), value));
	}
}

class RenderPass : public Pass
{
protected:
	std::map<std::string, std::vector<Graphics::DecoratedGraphicsObject*>> renderableObjects;
	bool terminal;
	virtual void initFrameBuffers(void) = 0;
	virtual void clearBuffers(void);
	virtual void configureGL(const std::string& programSignature);
	virtual void renderObjects(const std::string& programSignature);
	virtual void setupObjectwiseUniforms(const std::string& programSignature, int index);
	virtual void executeOwnBehaviour(void);
public:
	bool clearBuff = true;
	DecoratedFrameBuffer* frameBuffer;
	RenderPass(std::map<std::string, ShaderProgramPipeline*> shaderPipelines, std::string signature, DecoratedFrameBuffer* frameBuffer, bool terminal = false);
	~RenderPass();
	// TODO: Give the user the ability to apply renderable objects to any inner pass for rendering before the call
	virtual void clearRenderableObjects(const std::string& programSignature);
	virtual void clearRenderableObjects(const std::string& programSignature, const std::string& objectSignature) {};
	virtual void setRenderableObjects(std::map<std::string, std::vector<Graphics::DecoratedGraphicsObject*>> input);
	virtual void setRenderableObjects(std::vector<std::vector<Graphics::DecoratedGraphicsObject*>> input, std::string signature) {};
	virtual void addRenderableObjects(Graphics::DecoratedGraphicsObject* input, const std::string& programSignature);
	virtual void addRenderableObjects(Graphics::DecoratedGraphicsObject* input, const std::string& programSignature, const std::string& objectSignature) {};
	virtual void setProbe(const std::string& passSignature, const std::string& frameBufferSignature);
	virtual void setFrameBuffer(DecoratedFrameBuffer* fb);
	virtual void setTextureUniforms(const std::string& programSignature);
};

class GeometryPass : public RenderPass
{
protected:
	virtual void initFrameBuffers(void);
	virtual void clearBuffers(void);
	virtual void configureGL(const std::string& programSignature);
	virtual void setupObjectwiseUniforms(const std::string& programSignature, int index);
public:
	GeometryPass(std::map<std::string, ShaderProgramPipeline*> shaderPipelines,
				 std::string signature = "GEOMETRYPASS",
				 DecoratedFrameBuffer* frameBuffer = nullptr,
				 int pickingBufferCount = 0,
				 int stencilBufferCount = 0,
				 bool terminal = false);
	~GeometryPass() {};
	virtual void setupCamera(Camera* cam);
	virtual void setupOnHover(unsigned int id);
	virtual void setupVec4f(glm::vec4& input, std::string name);

	int pickingBufferCount;
	int stencilBufferCount;
};

class LightPass : public RenderPass
{
protected:
	virtual void initFrameBuffers(void);
	virtual void clearBuffers(void);
	virtual void configureGL(const std::string& programSignature);
public:
	LightPass(std::map<std::string, ShaderProgramPipeline*> shaderPipelines, bool terminal = false);
	~LightPass() {};
};


#pragma once
#include <iostream>
#include <time.h>
#include <glew.h>
#include <glfw3.h>

#include "GLFWWindowContext.h"
#include "ShaderProgramPipeline.h"
#include "ShaderProgram.h"
#include "LinearAlgebraUtils.h"
#include "SurfaceViewContext.h"

// GLEW and GLFW initialization. Projection and View matrix setup
int init() {
	WindowContext::context = new GLFWWindowContext(0.8, 0.8, "VISUAL COMPUTING");

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW\n" << std::endl;
		getchar();
		glfwTerminate();
		return -1;
	}

	//Check version of OpenGL and print
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

	// Enable depth test
	glDepthFunc(GL_LESS);

	glPointSize(3.0f);

	srand(time(NULL));

	ShaderProgram::getShaderProgram<VertexShaderProgram>("shaders\\gPass.VERTEXSHADER",
		{ std::tuple<const GLchar*, UniformType>("View", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("Projection", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("Model", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("selectedRef", ONEUI) }, "GPASSVS");
	
	ShaderProgram::getShaderProgram<VertexShaderProgram>("shaders\\EdgeGPass.VERTEXSHADER",
		{ std::tuple<const GLchar*, UniformType>("View", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("Projection", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("Model", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("selectedRef", ONEUI) }, "EDGEGPASSVS");

	ShaderProgram::getShaderProgram<VertexShaderProgram>("shaders\\vGPass.VERTEXSHADER",
		{ std::tuple<const GLchar*, UniformType>("View", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("Projection", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("Model", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("selectedRef", ONEUI) }, "VOLUMEGPASSVS");

	ShaderProgram::getShaderProgram<VertexShaderProgram>("shaders\\vGPass2.VERTEXSHADER",
		{ std::tuple<const GLchar*, UniformType>("View", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("Projection", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("Model", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("selectedRef", ONEUI),
		std::tuple<const GLchar*, UniformType>("inputColor", VECTOR4FV) }, "VOLUMEGPASSVS2");

	ShaderProgram::getShaderProgram<VertexShaderProgram>("shaders\\delaunaySphereGPass.VERTEXSHADER",
		{ std::tuple<const GLchar*, UniformType>("View", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("Projection", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("Model", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("selectedRef", ONEUI) }, "DSGPASSVS");

	ShaderProgram::getShaderProgram<VertexShaderProgram>("shaders\\lightPass.VERTEXSHADER", {}, "LPASSVS");
	
	ShaderProgram::getShaderProgram<VertexShaderProgram>("shaders\\pointGPass.VERTEXSHADER",
		{ std::tuple<const GLchar*, UniformType>("View", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("Projection", MATRIX4FV), 
		std::tuple<const GLchar*, UniformType>("Model", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("selectedRef", ONEUI) }, "POINTGPASSVS");

	ShaderProgram::getShaderProgram<VertexShaderProgram>("shaders\\facetGPass.VERTEXSHADER",
		{ std::tuple<const GLchar*, UniformType>("View", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("Projection", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("Model", MATRIX4FV),
		std::tuple<const GLchar*, UniformType>("selectedRef", ONEUI) }, "FACETGPASSVS");

	ShaderProgram::getShaderProgram<FragmentShaderProgram>("shaders\\gPass.FRAGMENTSHADER", {}, "GPASSFS");
	ShaderProgram::getShaderProgram<FragmentShaderProgram>("shaders\\gPass2.FRAGMENTSHADER", {}, "GPASSFS2");

	ShaderProgram::getShaderProgram<FragmentShaderProgram>("shaders\\lightPass.FRAGMENTSHADER",
		{ std::tuple<const GLchar*, UniformType>("gColors", TEXTURE),
		std::tuple<const GLchar*, UniformType>("gNormals", TEXTURE),
		std::tuple<const GLchar*, UniformType>("gPositions", TEXTURE) }, "LPASSFS");

	ShaderProgram::getShaderProgram<FragmentShaderProgram>("shaders\\stenciledLightPass.FRAGMENTSHADER", 
		{
			// TODO: ensure that order is somewhat preserved in map (maybe add an index?)
			std::tuple<const GLchar*, UniformType>("gColors", TEXTURE),
			std::tuple<const GLchar*, UniformType>("gNormals", TEXTURE),
			std::tuple<const GLchar*, UniformType>("gPositions", TEXTURE),
			std::tuple<const GLchar*, UniformType>("gSelected", TEXTURE),
			std::tuple<const GLchar*, UniformType>("gColors2", TEXTURE),
			std::tuple<const GLchar*, UniformType>("gNormals2", TEXTURE),
			std::tuple<const GLchar*, UniformType>("gPositions2", TEXTURE),
		 }, "SLPASSFS");

	ShaderProgram::getShaderProgram<GeometryShaderProgram>("shaders\\wireframeOverlay.GEOMETRYSHADER",
		{ /*std::tuple<const GLchar*, UniformType>("WIN_SCALE", TWOUI) */}, "WIREFRAMEOVERLAYGS");

	ShaderProgram::getShaderProgram<FragmentShaderProgram>("shaders\\wireframeOverlay.FRAGMENTSHADER", {}, "WIREFRAMEOVERLAYFS");

	ShaderProgram::getShaderProgram<GeometryShaderProgram>("shaders\\flatNormals.GEOMETRYSHADER",
		{ /*std::tuple<const GLchar*, UniformType>("WIN_SCALE", TWOUI) */ }, "FLATNORMALSGS");

	ShaderProgram::getShaderProgram<FragmentShaderProgram>("shaders\\flatNormals.FRAGMENTSHADER", {}, "FLATNORMALSFS");


	ShaderProgram::getShaderProgram<VertexShaderProgram>("GPASSVS")->attachToPipeline(ShaderProgramPipeline::getPipeline("A"));
	ShaderProgram::getShaderProgram<FragmentShaderProgram>("GPASSFS2")->attachToPipeline(ShaderProgramPipeline::getPipeline("A"));
	ShaderProgramPipeline::getPipeline("A")->alphaRendered = true;

	ShaderProgram::getShaderProgram<VertexShaderProgram>("GPASSVS")->attachToPipeline(ShaderProgramPipeline::getPipeline("A2"));
	ShaderProgram::getShaderProgram<GeometryShaderProgram>("FLATNORMALSGS")->attachToPipeline(ShaderProgramPipeline::getPipeline("A2"));
	ShaderProgram::getShaderProgram<FragmentShaderProgram>("FLATNORMALSFS")->attachToPipeline(ShaderProgramPipeline::getPipeline("A2"));

	ShaderProgram::getShaderProgram<VertexShaderProgram>("EDGEGPASSVS")->attachToPipeline(ShaderProgramPipeline::getPipeline("EdgeA"));
	ShaderProgram::getShaderProgram<FragmentShaderProgram>("GPASSFS")->attachToPipeline(ShaderProgramPipeline::getPipeline("EdgeA"));

	ShaderProgram::getShaderProgram<VertexShaderProgram>("LPASSVS")->attachToPipeline(ShaderProgramPipeline::getPipeline("B"));
	ShaderProgram::getShaderProgram<FragmentShaderProgram>("LPASSFS")->attachToPipeline(ShaderProgramPipeline::getPipeline("B"));

	ShaderProgram::getShaderProgram<VertexShaderProgram>("POINTGPASSVS")->attachToPipeline(ShaderProgramPipeline::getPipeline("C"));
	ShaderProgram::getShaderProgram<FragmentShaderProgram>("GPASSFS")->attachToPipeline(ShaderProgramPipeline::getPipeline("C"));

	ShaderProgram::getShaderProgram<VertexShaderProgram>("FACETGPASSVS")->attachToPipeline(ShaderProgramPipeline::getPipeline("D"));
	ShaderProgram::getShaderProgram<FragmentShaderProgram>("GPASSFS")->attachToPipeline(ShaderProgramPipeline::getPipeline("D"));

	ShaderProgram::getShaderProgram<VertexShaderProgram>("VOLUMEGPASSVS")->attachToPipeline(ShaderProgramPipeline::getPipeline("V1"));
	ShaderProgram::getShaderProgram<GeometryShaderProgram>("WIREFRAMEOVERLAYGS")->attachToPipeline(ShaderProgramPipeline::getPipeline("V1"));
	ShaderProgram::getShaderProgram<FragmentShaderProgram>("WIREFRAMEOVERLAYFS")->attachToPipeline(ShaderProgramPipeline::getPipeline("V1"));

	ShaderProgram::getShaderProgram<VertexShaderProgram>("VOLUMEGPASSVS2")->attachToPipeline(ShaderProgramPipeline::getPipeline("V2"));
	ShaderProgram::getShaderProgram<GeometryShaderProgram>("WIREFRAMEOVERLAYGS")->attachToPipeline(ShaderProgramPipeline::getPipeline("V2"));
	ShaderProgram::getShaderProgram<FragmentShaderProgram>("WIREFRAMEOVERLAYFS")->attachToPipeline(ShaderProgramPipeline::getPipeline("V2"));
	ShaderProgramPipeline::getPipeline("V2")->alphaRendered = true;

	ShaderProgram::getShaderProgram<VertexShaderProgram>("LPASSVS")->attachToPipeline(ShaderProgramPipeline::getPipeline("SB"));
	ShaderProgram::getShaderProgram<FragmentShaderProgram>("SLPASSFS")->attachToPipeline(ShaderProgramPipeline::getPipeline("SB"));

	LinearAlgebraUtils::init();
	
	return 1;
}

int main()
{
	if (init() < 0)
		return -1;

	SurfaceViewContext* context1 = new SurfaceViewContext();

	context1->setAsActiveContext();

	do {
		AbstractContext::activeContext->update();
		// Swap buffers
		glfwSwapBuffers(((GLFWWindowContext*)WindowContext::context)->window);
		glfwPollEvents();

	} while (glfwWindowShouldClose(((GLFWWindowContext*)WindowContext::context)->window) == 0);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
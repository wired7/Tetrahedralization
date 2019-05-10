#pragma once
#include "GLFWWindowContext.h"
#include <iostream>
#include <glew.h>
#include <glfw3.h>

GLFWWindowContext::GLFWWindowContext(float width, float height, const char* title)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		getchar();
	}

	auto screenRes = getScreenResolution();

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(screenRes.first * width, screenRes.second * height, title, NULL, NULL);
	if (window == NULL) {
		std::cerr << "Failed to open GLFW window." << std::endl;
		getchar();
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	context = this;
}


GLFWWindowContext::~GLFWWindowContext()
{
}

std::pair<int, int> GLFWWindowContext::getScreenResolution()
{
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	return std::make_pair(mode->width, mode->height);
}

std::pair<int, int> GLFWWindowContext::getSize()
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	return std::make_pair(width, height);
}

std::pair<int, int> GLFWWindowContext::getPos()
{
	int width, height;
	glfwGetWindowPos(window, &width, &height);

	return std::make_pair(width, height);
}

std::pair<double, double> GLFWWindowContext::getCursorPos()
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	return std::make_pair(xpos, ypos);
}
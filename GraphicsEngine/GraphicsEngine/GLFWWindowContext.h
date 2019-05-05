#pragma once
#include "WindowContext.h"

struct GLFWwindow;

class GLFWWindowContext : public WindowContext
{
public:
	GLFWwindow* window;
	GLFWWindowContext(int width, int height, const char* title);
	~GLFWWindowContext();
	virtual std::pair<int, int> getSize();
	virtual std::pair<double, double> getCursorPos();
};


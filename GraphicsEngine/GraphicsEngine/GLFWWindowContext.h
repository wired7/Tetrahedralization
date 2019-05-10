#pragma once
#include "WindowContext.h"

struct GLFWwindow;

class GLFWWindowContext : public WindowContext
{
public:
	GLFWwindow* window;
	GLFWWindowContext(float width, float height, const char* title);
	~GLFWWindowContext();
	std::pair<int, int> getScreenResolution() override;
	std::pair<int, int> getSize() override;
	std::pair<int, int> getPos() override;
	std::pair<double, double> getCursorPos() override;
};


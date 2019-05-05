#pragma once
#include "GLFWWindowContext.h"
#include <glew.h>
#include <glfw3.h>

class AbstractController
{
protected:
	GLFWwindow* window;
	void(*key_callback)(GLFWwindow*, int, int, int, int) = 0;
	void(*scroll_callback)(GLFWwindow*, double, double) = 0;
	void(*mouse_callback)(GLFWwindow*, int, int, int) = 0;
	void(*mousePos_callback)(GLFWwindow*, double, double) = 0;
	void(*windowResize_callback)(GLFWwindow*, int, int) = 0;
public:
	AbstractController() : window(((GLFWWindowContext*)WindowContext::context)->window) {};
};

template<class T, class S> class Controller : public AbstractController
{
protected:
	static T* controller;
	Controller();
	~Controller() {};
public:
	S* context;
	static T* getController();
	void setContext(S* context);
	void setController();
};

#pragma region ControllerTemplate
template<class T, class S> T* Controller<T, S>::controller = nullptr;

template<class T, class S> Controller<T, S>::Controller() : AbstractController()
{
}

template<class T, class S> void Controller<T, S>::setController()
{
	controller = (T*)this;

	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetCursorPosCallback(window, mousePos_callback);
	glfwSetWindowSizeCallback(window, windowResize_callback);
}

template<class T, class S> T* Controller<T, S>::getController()
{
	if (controller == nullptr)
	{
		controller = new T();
	}

	return controller;
}

template<class T, class S> void Controller<T, S>::setContext(S* context)
{
	this->context = context;
}
#pragma endregion
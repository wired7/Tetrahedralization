#pragma once
#include "Controller.h"
#include "Camera.h"
#include <glm.hpp>

template<class T, class S> class FPSCameraController : virtual public Controller<T, S>
{
protected:
	FPSCameraController() {};
	virtual ~FPSCameraController() {};
public:
	bool firstMouse = false;
	bool cameraDrag = false;
	float lastX = 0;
	float lastY = 0;

	virtual void scrollMove(double xOffset, double yOffset);
	virtual void rotateHead(FPSCamera* cam, double xOffset, double yOffset);
	virtual void mousePosRotation(double xpos, double ypos);
	virtual void keyboardControls(GLFWwindow* window, int key, int action);
	virtual void mouseClickControls(int button, int action);
};

template<class T, class S> void FPSCameraController<T, S>::scrollMove(double xOffset, double yOffset)
{
	context->cameras[0]->moveInDirection(glm::vec3(-glm::sign(xOffset) * 4, 0, glm::sign(yOffset) * 4));
};

template<class T, class S> void FPSCameraController<T, S>::rotateHead(FPSCamera* cam, double xOffset, double yOffset)
{
	if (cam->freeRotation)
	{
		cam->rotate(xOffset, yOffset);
	}
};

template<class T, class S> void FPSCameraController<T, S>::mousePosRotation(double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	auto cam = context->cameras[0];
	/*		if (cameraDrag)
	{
	auto cosine = -cam->lookAtVector;
	auto sine = -cam->rightVector;
	float angle = atan2(xoffset, 3.0f);
	glm::vec3 targetDir = 3.0f * (-cosine * sin(angle) + sine * cos(angle));
	bool freeRotation = cam->freeRotation;
	cam->freeRotation = true;
	cam->yaw += 90 - glm::degrees(angle / 2);
	cam->freeRotation = freeRotation;
	cam->move(targetDir);
	}
	else
	{*/
	bool freeRotation = cam->freeRotation;
	cam->freeRotation = freeRotation || cameraDrag;
	rotateHead(cam, xoffset, yoffset);
	cam->freeRotation = freeRotation;
	//		}
};

template<class T, class S> void FPSCameraController<T, S>::keyboardControls(GLFWwindow* window, int key, int action)
{
	// camera controls
	if (key == GLFW_KEY_F1 && action == GLFW_RELEASE)
	{
		bool& freeRotation = context->cameras[0]->freeRotation;

		freeRotation = !freeRotation;

		if (freeRotation)
		{
			glfwSetInputMode(((GLFWWindowContext*)WindowContext::context)->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else
		{
			glfwSetInputMode(((GLFWWindowContext*)WindowContext::context)->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
	{
		context->cameras[0]->changeSpeed();
	}

	if (key == GLFW_KEY_W)
	{
		if (action == GLFW_PRESS)
			context->cameras[0]->addVelocity(glm::vec3(0, 0, 1));
		if (action == GLFW_RELEASE)
			context->cameras[0]->addVelocity(glm::vec3(0, 0, -1));
	}

	if (key == GLFW_KEY_S)
	{
		if (action == GLFW_PRESS)
			context->cameras[0]->addVelocity(glm::vec3(0, 0, -1));
		if (action == GLFW_RELEASE)
			context->cameras[0]->addVelocity(glm::vec3(0, 0, 1));
	}

	if (key == GLFW_KEY_A)
	{
		if (action == GLFW_PRESS)
			context->cameras[0]->addVelocity(glm::vec3(-1, 0, 0));
		if (action == GLFW_RELEASE)
			context->cameras[0]->addVelocity(glm::vec3(1, 0, 0));
	}

	if (key == GLFW_KEY_D)
	{
		if (action == GLFW_PRESS)
			context->cameras[0]->addVelocity(glm::vec3(1, 0, 0));
		if (action == GLFW_RELEASE)
			context->cameras[0]->addVelocity(glm::vec3(-1, 0, 0));
	}

	if (key == GLFW_KEY_SPACE)
	{
		if (action == GLFW_PRESS)
			context->cameras[0]->addVelocity(glm::vec3(0, 1, 0));
		if (action == GLFW_RELEASE)
			context->cameras[0]->addVelocity(glm::vec3(0, -1, 0));
	}

	if (key == GLFW_KEY_LEFT_CONTROL)
	{
		if (action == GLFW_PRESS)
			context->cameras[0]->addVelocity(glm::vec3(0, -1, 0));
		if (action == GLFW_RELEASE)
			context->cameras[0]->addVelocity(glm::vec3(0, 1, 0));
	}
};

template<class T, class S> void FPSCameraController<T, S>::mouseClickControls(int button, int action)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS)
		{
			cameraDrag = true;
		}
		else if (action == GLFW_RELEASE)
		{
			cameraDrag = false;
		}
	}
}
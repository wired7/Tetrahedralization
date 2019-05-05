#pragma once
#include "Camera.h"
#include "glm.hpp"

namespace FPSCameraControls
{
	static void moveCamera(FPSCamera* cam, float xOffset, float yOffset)
	{
		if (cam->usingCamera)
		{
			cam->processMouseMovement(xOffset, yOffset);
		}
	};

	static void moveCamera(FPSCamera* cam, glm::vec3 dir) {
		cam->processKeyInput(dir);
		cam->update();
	};

	template <class T> static void cameraKeyboardControls(T* context, int key, int action)
	{
		// camera controls
		if (key == GLFW_KEY_F1 && action == GLFW_RELEASE)
		{
			bool& usingCamera = context->cameras[0]->usingCamera;

			usingCamera = !usingCamera;

			if (usingCamera)
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
				context->cameras[0]->velocity += glm::vec3(0, 0, 1);
			if (action == GLFW_RELEASE)
				context->cameras[0]->velocity -= glm::vec3(0, 0, 1);
		}

		if (key == GLFW_KEY_S)
		{
			if (action == GLFW_PRESS)
				context->cameras[0]->velocity -= glm::vec3(0, 0, 1);
			if (action == GLFW_RELEASE)
				context->cameras[0]->velocity += glm::vec3(0, 0, 1);
		}

		if (key == GLFW_KEY_A)
		{
			if (action == GLFW_PRESS)
				context->cameras[0]->velocity += glm::vec3(-1, 0, 0);
			if (action == GLFW_RELEASE)
				context->cameras[0]->velocity -= glm::vec3(-1, 0, 0);
		}

		if (key == GLFW_KEY_D)
		{
			if (action == GLFW_PRESS)
				context->cameras[0]->velocity += glm::vec3(1, 0, 0);
			if (action == GLFW_RELEASE)
				context->cameras[0]->velocity -= glm::vec3(1, 0, 0);
		}

		if (key == GLFW_KEY_SPACE)
		{
			if (action == GLFW_PRESS)
				context->cameras[0]->velocity += glm::vec3(0, 1, 0);
			if (action == GLFW_RELEASE)
				context->cameras[0]->velocity -= glm::vec3(0, 1, 0);
		}

		if (key == GLFW_KEY_LEFT_CONTROL)
		{
			if (action == GLFW_PRESS)
				context->cameras[0]->velocity += glm::vec3(0, -1, 0);
			if (action == GLFW_RELEASE)
				context->cameras[0]->velocity -= glm::vec3(0, -1, 0);
		}
	};

	template <class T> static void mousePositionControls(T* controller, double xpos, double ypos)
	{
		if (controller->firstMouse)
		{
			controller->lastX = xpos;
			controller->lastY = ypos;
			controller->firstMouse = false;
		}

		float xoffset = xpos - controller->lastX;
		float yoffset = controller->lastY - ypos; // reversed since y-coordinates go from bottom to top

		controller->lastX = xpos;
		controller->lastY = ypos;

		//camera->ProcessMouseMovement(xoffset, yoffset);

		moveCamera(controller->context->cameras[0], xoffset, yoffset);
	};
}
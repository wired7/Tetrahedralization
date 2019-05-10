#pragma once
#include "SurfaceViewController.h"
#include "PointSamplingContext.h"

SurfaceViewController::SurfaceViewController()
{
}

SurfaceViewController::~SurfaceViewController()
{
}

void SurfaceViewController::kC(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
	{
		if (controller->context->nextContext == nullptr)
		{
			auto geo = controller->context->geometries["VOLUME"];
			auto cam = controller->context->cameras[0];
			auto refMan = controller->context->refMan;

			controller->context->nextContext = new PointSamplingContext(geo, cam, refMan);
			((PointSamplingContext*)controller->context->nextContext)->prevContext = controller->context;
		}

		((PointSamplingContext*)controller->context->nextContext)->setAsActiveContext();
	}

	controller->keyboardRendering(key, action);
	controller->keyboardControls(window, key, action);
	controller->updatePicker(window);
}

void SurfaceViewController::sC(GLFWwindow* window, double xOffset, double yOffset)
{
	controller->scrollMove(xOffset, yOffset);
	controller->updatePicker(window);
}

void SurfaceViewController::mC(GLFWwindow* window, int button, int action, int mods)
{
	controller->mouseClickControls(button, action);
}

void SurfaceViewController::mPC(GLFWwindow* window, double xpos, double ypos)
{
	controller->mousePosRotation(xpos, ypos);
	controller->updatePicker(window);
}

void SurfaceViewController::wRC(GLFWwindow*, int a, int b)
{
}
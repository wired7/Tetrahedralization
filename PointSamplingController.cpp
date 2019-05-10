#pragma once
#include "PointSamplingController.h"
#include "SurfaceViewContext.h"
#include "TetrahedralizationContext.h"

PointSamplingController::PointSamplingController()
{
	surfaceRendering = false;
}

PointSamplingController::~PointSamplingController()
{
}

void PointSamplingController::kC(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (controller->context->readyToAdvance)
	{
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		{
			if (controller->context->nextContext == nullptr)
			{
				auto geo = controller->context->geometries["SURFACE"];
				auto geoPoints = controller->context->geometries["VERTICES"];
				auto &points = controller->context->points;
				auto cam = controller->context->cameras[0];
				auto refMan = controller->context->refMan;

				controller->context->nextContext = new TetrahedralizationContext(geo, geoPoints, points, cam, refMan);
				((TetrahedralizationContext*)controller->context->nextContext)->prevContext = controller->context;
			}

			((TetrahedralizationContext*)controller->context->nextContext)->setAsActiveContext();
		}
	}

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
	{
		dynamic_cast<SurfaceViewContext*>(controller->context->prevContext)->setAsActiveContext();
	}

	controller->keyboardRendering(key, action);
	controller->keyboardControls(window, key, action);
	controller->updatePicker(window);
}

void PointSamplingController::sC(GLFWwindow* window, double xOffset, double yOffset)
{
	controller->scrollMove(xOffset, yOffset);
	controller->updatePicker(window);
}

void PointSamplingController::mC(GLFWwindow* window, int button, int action, int mods)
{
	controller->mouseClickControls(button, action);
}

void PointSamplingController::mPC(GLFWwindow* window, double xpos, double ypos)
{
	controller->mousePosRotation(xpos, ypos);
	controller->updatePicker(window);
}

void PointSamplingController::wRC(GLFWwindow* window, int a, int b)
{
}

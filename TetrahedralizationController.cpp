#pragma once
#include "TetrahedralizationController.h"
#include "ClusteringStageContext.h"
#include "PointSamplingContext.h"

#define REFRESH_RATE 0.1

TetrahedralizationController::TetrahedralizationController()
{
}

TetrahedralizationController::~TetrahedralizationController()
{
}

void TetrahedralizationController::kC(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (controller->context->readyToAdvance)
	{
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		{
			if (controller->context->nextContext == nullptr)
			{
				auto geo = controller->context->geometries["VOLUMES"];
				auto cam = controller->context->cameras[0];

				controller->context->nextContext = new ClusteringStageContext(geo, controller->context->manifold,
					controller->context->positions,
					controller->context->simplexToBufferInstanceMap, controller->context->refMan, cam);
				((ClusteringStageContext*)controller->context->nextContext)->prevContext = controller->context;
			}

			((ClusteringStageContext*)controller->context->nextContext)->setAsActiveContext();
		}
	}

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
	{
		((PointSamplingContext*)controller->context->prevContext)->setAsActiveContext();
	}

	controller->keyboardRendering(key, action);
	controller->keyboardControls(window, key, action);
	controller->updatePicker(window);
}

void TetrahedralizationController::sC(GLFWwindow* window, double xOffset, double yOffset)
{
	controller->scrollMove(xOffset, yOffset);
	controller->updatePicker(window);
}

void TetrahedralizationController::mC(GLFWwindow* window, int button, int action, int mods)
{
	controller->mouseClickControls(button, action);
}

void TetrahedralizationController::mPC(GLFWwindow* window, double xpos, double ypos)
{
	controller->mousePosRotation(xpos, ypos);
	controller->updatePicker(window);
}

void TetrahedralizationController::wRC(GLFWwindow* window, int a, int b)
{
}



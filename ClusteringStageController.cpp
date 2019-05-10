#pragma once
#include "ClusteringStageController.h"
#include "TetrahedralizationContext.h"
#include "ClusteringStageContext.h"
#include "OpenCLContext.h"

#define MAX_SEPARATION_DISTANCE 20

ClusteringStageController::ClusteringStageController()
{
}

ClusteringStageController::~ClusteringStageController()
{
}

void ClusteringStageController::kC(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		((TetrahedralizationContext*)controller->context->prevContext)->setAsActiveContext();
	}

	if (key == GLFW_KEY_UP && action != GLFW_RELEASE)
	{
		ClusteringStageContext* context = controller->context;
		if (context->separationDistanceBuffer->bufferData[0] < MAX_SEPARATION_DISTANCE)
		{
			context->separationDistanceBuffer->bufferData[0] += 0.2f;
			context->updateDistanceBuffer = true;
		}
	}

	if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE)
	{
		ClusteringStageContext* context = controller->context;
		if (context->separationDistanceBuffer->bufferData[0] > 0)
		{
			context->separationDistanceBuffer->bufferData[0] -= 0.2f;
			context->updateDistanceBuffer = true;
		}
	}

	controller->keyboardRendering(key, action);
	controller->keyboardControls(window, key, action);
	controller->updatePicker(window, "GEOMETRYPASS0");
}

void ClusteringStageController::sC(GLFWwindow* window, double xOffset, double yOffset)
{
	controller->scrollMove(xOffset, yOffset);
	controller->updatePicker(window, "GEOMETRYPASS0");
}

void ClusteringStageController::mC(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
	{
		if (!controller->context->busy)
		{
			auto cursorPos = WindowContext::context->getCursorPos();
			double xpos = cursorPos.first;
			double ypos = cursorPos.second;

			auto gPass = (GeometryPass*)controller->context->passRootNode->signatureLookup("GEOMETRYPASS0");
			auto stencil = (PickingBuffer*)gPass->frameBuffer->signatureLookup("PICKING0");

			if (stencil)
			{
				auto widthHeight = WindowContext::context->getSize();
				auto data = stencil->getValues(xpos, widthHeight.second - ypos);
				int d = data[0];
				auto instance = controller->getPickingID(gPass, xpos, ypos);
				auto tetraTransform = controller->context->refMan->getInstance(instance);
				controller->context->calculateGeodesicFromTetra(controller->context->bufferInstanceToSimplexMap[tetraTransform.second]);
				delete[] data;
			}

			controller->context->dirty = true;
		}
	}

	controller->mouseClickControls(button, action);
}

void ClusteringStageController::mPC(GLFWwindow* window, double xpos, double ypos)
{
	controller->mousePosRotation(xpos, ypos);
	controller->updatePicker(window, "GEOMETRYPASS0");
}

void ClusteringStageController::wRC(GLFWwindow* window, int a, int b)
{
}
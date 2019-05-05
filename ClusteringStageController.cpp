#pragma once
#include "ClusteringStageController.h"
#include "SurfaceViewController.h"
#include "FPSCameraControls.h"
#include "TetrahedralizationContext.h"
#include "ClusteringStageContext.h"
#include "ReferencedGraphicsObject.h"
#include "FrameBuffer.h"
#include "OpenCLContext.h"
#include <glew.h>
#include "glfw3.h"

#define MAX_SEPARATION_DISTANCE 20

ClusteringStageController::ClusteringStageController()
{
	key_callback = kC;
	scroll_callback = sC;
	mouse_callback = mC;
	mousePos_callback = mPC;
	windowResize_callback = wRC;
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
			context->dirty = true;
		}
	}

	if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE)
	{
		ClusteringStageContext* context = controller->context;
		if (context->separationDistanceBuffer->bufferData[0] > 0)
		{
			context->separationDistanceBuffer->bufferData[0] -= 0.2f;
			context->updateDistanceBuffer = true;
			context->dirty = true;
		}
	}

	FPSCameraControls::cameraKeyboardControls<ClusteringStageContext>(controller->context, key, action);
}

void ClusteringStageController::sC(GLFWwindow* window, double xOffset, double yOffset)
{

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

			auto stencil = (PickingBuffer*)((GeometryPass*)controller->context->passRootNode->signatureLookup("GEOMETRYPASS0"))->frameBuffer->signatureLookup("PICKING0");

			if (stencil)
			{
				auto widthHeight = WindowContext::context->getSize();

				auto data = stencil->getValues(xpos, widthHeight.second - ypos);

				int d = data[0];

				auto instance = SurfaceViewController::getPickingID((GeometryPass*)controller->context->passRootNode->signatureLookup("GEOMETRYPASS0"),
					xpos, ypos);

				auto tetraTransform = controller->context->refMan->getInstance(instance);

				controller->context->calculateGeodesicFromTetra(controller->context->bufferInstanceToSimplexMap[tetraTransform.second]);

				delete[] data;
			}

			controller->context->dirty = true;
		}
	}
}

void ClusteringStageController::mPC(GLFWwindow* window, double xpos, double ypos)
{
	FPSCameraControls::mousePositionControls<ClusteringStageController>(controller, xpos, ypos);

	SurfaceViewController::getPickingID((GeometryPass*)controller->context->passRootNode->signatureLookup("GEOMETRYPASS0"), xpos, ypos);

	controller->context->dirty = true;
}

void ClusteringStageController::wRC(GLFWwindow* window, int a, int b)
{
	controller->context->cameras[0]->update();
	controller->context->dirty = true;
}
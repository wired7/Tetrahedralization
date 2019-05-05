#pragma once
#include "PointSamplingController.h"
#include "SurfaceViewController.h"
#include "TetrahedralizationContext.h"
#include "FPSCameraControls.h"
#include <glew.h>

PointSamplingController::PointSamplingController()
{
	key_callback = kC;
	scroll_callback = sC;
	mouse_callback = mC;
	mousePos_callback = mPC;
	windowResize_callback = wRC;
}

PointSamplingController::~PointSamplingController()
{
}

void PointSamplingController::kC(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		auto geo0 = controller->context->geometries["SURFACE"];
		auto geo2 = controller->context->geometries["POINTS"];
		auto pass = (GeometryPass*)controller->context->passRootNode;

		pass->clearRenderableObjects("A");
		if (controller->surfaceRendering)
		{
			pass->addRenderableObjects(geo0, "A");
		}

		controller->surfaceRendering ^= true;
	}

	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		if (controller->context->readyToAdvance)
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

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		if (controller->context->readyToAdvance)
		{
			((SurfaceViewContext*)controller->context->prevContext)->setAsActiveContext();
		}
	}

	FPSCameraControls::cameraKeyboardControls(controller->context, key, action);

	controller->context->dirty = true;
}

void PointSamplingController::sC(GLFWwindow* window, double xOffset, double yOffset)
{
	FPSCamera* cam = reinterpret_cast<FPSCamera*>(controller->context->cameras[0]);
	FPSCameraControls::moveCamera(cam, glm::vec3(-glm::sign(xOffset), 0, glm::sign(yOffset)));

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	SurfaceViewController::getPickingID((GeometryPass*)controller->context->passRootNode, xpos, ypos);

	controller->context->dirty = true;
}

void PointSamplingController::mC(GLFWwindow* window, int button, int action, int mods)
{
}

void PointSamplingController::mPC(GLFWwindow* window, double xpos, double ypos)
{
	FPSCameraControls::mousePositionControls(controller, xpos, ypos);

	SurfaceViewController::getPickingID((GeometryPass*)controller->context->passRootNode, xpos, ypos);
	controller->context->dirty = true;
}

void PointSamplingController::wRC(GLFWwindow* window, int a, int b)
{
	controller->context->cameras[0]->update();
	controller->context->dirty = true;
}

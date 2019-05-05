#pragma once
#include "TetrahedralizationController.h"
#include "SurfaceViewController.h"
#include "FPSCameraControls.h"
#include "ClusteringStageContext.h"
#include "PointSamplingContext.h"
#include "Pass.h"
#include <thread>
#include <glew.h>

#define REFRESH_RATE 0.1
bool TetrahedralizationController::stopUpdate = false;
bool TetrahedralizationController::isWorking = false;
TetrahedralizationController::TetrahedralizationController() {
	key_callback = kC;
	scroll_callback = sC;
	mouse_callback = mC;
	mousePos_callback = mPC;
	windowResize_callback = wRC;
}
TetrahedralizationController::~TetrahedralizationController() {

}

void TetrahedralizationController::kC(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		controller->volumeRendering ^= true;

		auto pass = (GeometryPass*)controller->context->passRootNode;

		pass->clearRenderableObjects("D");
		if (controller->volumeRendering)
		{
			auto geo0 = controller->context->geometries["VOLUMES"];
			pass->addRenderableObjects(geo0, "D");
		}

/*		pass->clearRenderableObjects("A");
		if (controller->surfaceRendering)
		{
			auto geo1 = controller->context->geometries["SURFACE"];
			pass->addRenderableObjects(geo1, "A");
		}*/
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		stopUpdate = true;
	}

	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
/*		controller->facetRendering ^= true;

		auto pass = (GeometryPass*)controller->context->passRootNode;

		pass->clearRenderableObjects("D");
		if (controller->facetRendering)
		{
			auto geo0 = controller->context->geometries["FACETS"];
			pass->addRenderableObjects(geo0, "D");
		}*/
	}

	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
/*		controller->edgeRendering ^= true;

		auto pass = (GeometryPass*)controller->context->passRootNode;

		pass->clearRenderableObjects("EdgeA");
		if (controller->edgeRendering)
		{
			auto geo0 = controller->context->geometries["EDGES"];
			pass->addRenderableObjects(geo0, "EdgeA");
		}*/
	}

	if (key == GLFW_KEY_V && action == GLFW_PRESS)
	{
		controller->pointRendering ^= true;

		auto pass = (GeometryPass*)controller->context->passRootNode;

		pass->clearRenderableObjects("C");
		if (controller->pointRendering)
		{
			auto geo0 = controller->context->geometries["POINTS"];
			pass->addRenderableObjects(geo0, "C");
		}
	}
	
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
	
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
	{
			((PointSamplingContext*)controller->context->prevContext)->setAsActiveContext();
	}

	FPSCameraControls::cameraKeyboardControls<TetrahedralizationContext>(controller->context, key, action);
	
	controller->context->dirty = true;
	
}

void TetrahedralizationController::sC(GLFWwindow* window, double xOffset, double yOffset)
{

	//SurfaceViewController::cameraMovement(cam, xOffset, yOffset);

	//controller->context->dirty = true;
}

void TetrahedralizationController::mC(GLFWwindow* window, int button, int action, int mods)
{
}

void TetrahedralizationController::mPC(GLFWwindow* window, double xpos, double ypos)
{
	FPSCameraControls::mousePositionControls<TetrahedralizationController>(controller, xpos, ypos);
	
	SurfaceViewController::getPickingID((GeometryPass*)controller->context->passRootNode, xpos, ypos);
	controller->context->dirty = true;
}

void TetrahedralizationController::wRC(GLFWwindow* window, int a, int b)
{
	controller->context->cameras[0]->update();
	controller->context->dirty = true;
}



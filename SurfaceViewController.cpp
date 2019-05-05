#pragma once
#include "glew.h"
#include "SurfaceViewController.h"
#include "PointSamplingContext.h"
#include "FPSCameraControls.h"
#include "FrameBuffer.h"

SurfaceViewController::SurfaceViewController()
{
	key_callback = kC;
	scroll_callback = sC;
	mouse_callback = mC;
	mousePos_callback = mPC;
	windowResize_callback = wRC;
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

	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		controller->edgeRendering ^= true;

		auto pass = (GeometryPass*)controller->context->passRootNode;

		if (controller->edgeRendering)
		{
			auto geo1 = controller->context->geometries["EDGES"];
			pass->addRenderableObjects(geo1, "EdgeA");
		}
		else
		{
			pass->clearRenderableObjects("EdgeA");
		}
	}

	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		controller->surfaceRendering ^= true;

		auto pass = (GeometryPass*)controller->context->passRootNode;

		if (controller->surfaceRendering)
		{
			auto geo0 = controller->context->geometries[0];
			pass->addRenderableObjects(geo0, 0);
		}
		else
		{
			pass->clearRenderableObjects(0);
		}
	}

	if (key == GLFW_KEY_V && action == GLFW_PRESS)
	{
		controller->pointRendering ^= true;

		auto pass = (GeometryPass*)controller->context->passRootNode;

		if (controller->pointRendering)
		{
			auto geo0 = controller->context->geometries["VERTICES"];
			pass->addRenderableObjects(geo0, "C");
		}
		else
		{
			pass->clearRenderableObjects("C");
		}
	}

	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		controller->facetRendering ^= true;

		auto pass = (GeometryPass*)controller->context->passRootNode;

		if (controller->facetRendering)
		{
			auto geo0 = controller->context->geometries["FACETS"];
			pass->addRenderableObjects(geo0, "D");
		}
		else
		{
			pass->clearRenderableObjects("D");
		}
	}

	FPSCameraControls::cameraKeyboardControls<SurfaceViewContext>(controller->context, key, action);

	controller->context->dirty = true;
}

void SurfaceViewController::sC(GLFWwindow* window, double xOffset, double yOffset)
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	getPickingID((GeometryPass*)controller->context->passRootNode, xpos, ypos);

	controller->context->dirty = true;
}

void SurfaceViewController::mC(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{

	}
	if ((button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) && action == GLFW_RELEASE)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

/*	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
	{
		double xpos;
		double ypos;
		glfwGetCursorPos(WindowContext::window, &xpos, &ypos);
		unsigned int index = SurfaceViewController::getPickingID((GeometryPass*)controller->context->passRootNode, xpos, ypos);
		cout << endl << index << endl;
		((SurfaceViewContext*)controller->context)->facetBFS(((SurfaceViewContext*)controller->context)->vMesh->meshes[0]->facets[0], 4);
	}*/
}

void SurfaceViewController::mPC(GLFWwindow* window, double xpos, double ypos)
{
	FPSCameraControls::mousePositionControls<SurfaceViewController>(controller, xpos, ypos);

	getPickingID((GeometryPass*)controller->context->passRootNode, xpos, ypos);
	controller->context->dirty = true;
}

void SurfaceViewController::wRC(GLFWwindow*, int a, int b)
{
	controller->context->cameras[0]->update();
	controller->context->dirty = true;
}

unsigned int SurfaceViewController::getPickingID(GeometryPass* gP, double xpos, double ypos, std::string signature)
{
	auto widthHeight = WindowContext::context->getSize();
	
	auto picking = (PickingBuffer*)gP->frameBuffer->signatureLookup(signature);
	auto data = picking->getValues(xpos, widthHeight.second - ypos);
	gP->setupOnHover(data[0]);
	int d = data[0];

	delete[] data;

	return d;
}
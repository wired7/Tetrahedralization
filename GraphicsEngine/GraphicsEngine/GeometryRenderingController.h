#pragma once
#include "Controller.h"
#include <string>
#include "Pass.h"
#include "FrameBuffer.h"
#include "ReferencedGraphicsObject.h"
#include <glew.h>
#include <glfw3.h>

template<class T, class S>
class GeometryRenderingController : virtual public Controller<T, S>
{
protected:
	bool pointRendering = true;
	bool edgeRendering = true;
	bool surfaceRendering = true;
	bool volumeRendering = true;

	unsigned int lastPick;

	virtual unsigned int getPickingID(GeometryPass* gP, double xpos, double ypos,
									  std::string signature = "PICKING0");
	virtual void updatePicker(GLFWwindow* window, std::string passSignature = "GEOMETRYPASS");
	virtual void keyboardRendering(int key, int action);
};

template<class T, class S>
unsigned int GeometryRenderingController<T, S>::getPickingID(GeometryPass* gP, double xpos,
															 double ypos, std::string signature)
{
	auto widthHeight = WindowContext::context->getSize();

	auto picking = (PickingBuffer*)gP->frameBuffer->signatureLookup(signature);
	auto data = picking->getValues(xpos, widthHeight.second - ypos);
	gP->setupOnHover(data[0]);
	int d = data[0];

	delete[] data;

	return d;
};

template<class T, class S>
void GeometryRenderingController<T, S>::updatePicker(GLFWwindow* window, std::string passSignature)
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	auto gP = (GeometryPass*)context->passRootNode->signatureLookup(passSignature);
	unsigned int currentPick = getPickingID(gP, xpos, ypos);
	int maxCount = controller->context->refMan->count;
	if (lastPick != currentPick && (currentPick < maxCount || lastPick < maxCount))
	{
		controller->context->dirty = true;
	}

	lastPick = currentPick;
};

template<class T, class S>
void GeometryRenderingController<T, S>::keyboardRendering(int key, int action)
{
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		edgeRendering ^= true;

		if (edgeRendering)
		{
			context->addEdges();
		}
		else
		{
			context->removeEdges();
		}

		context->dirty = true;
	}

	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		volumeRendering ^= true;

		if (volumeRendering)
		{
			context->addVolumes();
		}
		else
		{
			context->removeVolumes();
		}

		context->dirty = true;
	}

	if (key == GLFW_KEY_V && action == GLFW_PRESS)
	{
		pointRendering ^= true;

		if (pointRendering)
		{
			context->addVertices();
		}
		else
		{
			context->removeVertices();
		}

		context->dirty = true;
	}

	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		surfaceRendering ^= true;

		if (surfaceRendering)
		{
			context->addSurfaces();
		}
		else
		{
			context->removeSurfaces();
		}

		context->dirty = true;
	}
}
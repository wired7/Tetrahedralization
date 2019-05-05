#pragma once
#include "Controller.h"
#include "SurfaceViewContext.h"
#include "Camera.h"

class SurfaceViewContext;
class GeometryPass;
class FPSCamera;

class SurfaceViewController : public Controller<SurfaceViewController, SurfaceViewContext>
{
public:
	bool pointRendering = true;
	bool edgeRendering = true;
	bool surfaceRendering = true;
	bool facetRendering = true;

	bool firstMouse = false;
	float lastX = 0;
	float lastY = 0;

	SurfaceViewController();
	~SurfaceViewController();
	static void kC(GLFWwindow*, int, int, int, int);
	static void sC(GLFWwindow*, double, double);
	static void mC(GLFWwindow*, int, int, int);
	static void mPC(GLFWwindow*, double, double);
	static void wRC(GLFWwindow*, int, int);

	static unsigned int getPickingID(GeometryPass* gP, double xpos, double ypos, std::string signature = "PICKING0");
};

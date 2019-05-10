#pragma once
#include "FPSCameraController.h"
#include "PointSamplingContext.h"
#include "GeometryRenderingController.h"

class PointSamplingContext;
struct GLFWwindow;

class PointSamplingController : public FPSCameraController<PointSamplingController, PointSamplingContext>,
								public GeometryRenderingController<PointSamplingController, PointSamplingContext>
{
public:
	PointSamplingController();
	~PointSamplingController();
	static void kC(GLFWwindow*, int, int, int, int);
	static void sC(GLFWwindow*, double, double);
	static void mC(GLFWwindow*, int, int, int);
	static void mPC(GLFWwindow*, double, double);
	static void wRC(GLFWwindow*, int, int);
};

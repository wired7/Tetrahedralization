#pragma once
#include "Controller.h"
#include "PointSamplingContext.h"

class PointSamplingContext;
struct GLFWwindow;

class PointSamplingController : public Controller<PointSamplingController, PointSamplingContext>
{
public:
	bool surfaceRendering = false;

	bool firstMouse = false;
	float lastX = 0;
	float lastY = 0;

	PointSamplingController();
	~PointSamplingController();
	static void kC(GLFWwindow*, int, int, int, int);
	static void sC(GLFWwindow*, double, double);
	static void mC(GLFWwindow*, int, int, int);
	static void mPC(GLFWwindow*, double, double);
	static void wRC(GLFWwindow*, int, int);
};

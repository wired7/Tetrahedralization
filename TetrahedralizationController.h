#pragma once
#include "Controller.h"
#include "TetrahedralizationContext.h"

class TetrahedralizationContext;
struct GLFWwindow;

class TetrahedralizationController : public Controller<TetrahedralizationController, TetrahedralizationContext>
{
public:
	bool surfaceRendering = true;
	bool edgeRendering = true;
	bool pointRendering = true;
	bool facetRendering = true;
	bool volumeRendering = true;
	int numberOfIterations = 1;

	bool firstMouse = false;
	static bool stopUpdate;
	static bool isWorking;

	float lastX = 0;
	float lastY = 0;

	TetrahedralizationController();
	~TetrahedralizationController();
	static void kC(GLFWwindow*, int, int, int, int);
	static void sC(GLFWwindow*, double, double);
	static void mC(GLFWwindow*, int, int, int);
	static void mPC(GLFWwindow*, double, double);
	static void wRC(GLFWwindow*, int, int);
};
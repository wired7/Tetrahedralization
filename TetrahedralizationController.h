#pragma once
#include "FPSCameraController.h"
#include "TetrahedralizationContext.h"
#include "GeometryRenderingController.h"

class TetrahedralizationContext;
struct GLFWwindow;

class TetrahedralizationController : public FPSCameraController<TetrahedralizationController, TetrahedralizationContext>,
									 public GeometryRenderingController<TetrahedralizationController, TetrahedralizationContext>
{
public:
	bool volumeRendering = true;
	TetrahedralizationController();
	~TetrahedralizationController();
	static void kC(GLFWwindow*, int, int, int, int);
	static void sC(GLFWwindow*, double, double);
	static void mC(GLFWwindow*, int, int, int);
	static void mPC(GLFWwindow*, double, double);
	static void wRC(GLFWwindow*, int, int);
};
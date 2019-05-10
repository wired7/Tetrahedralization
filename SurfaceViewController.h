#pragma once
#include "FPSCameraController.h"
#include "SurfaceViewContext.h"
#include "GeometryRenderingController.h"

class SurfaceViewContext;
class GeometryPass;
class FPSCamera;

class SurfaceViewController : public FPSCameraController<SurfaceViewController, SurfaceViewContext>,
							  public GeometryRenderingController<SurfaceViewController, SurfaceViewContext>
{
public:
	SurfaceViewController();
	~SurfaceViewController();
	static void kC(GLFWwindow*, int, int, int, int);
	static void sC(GLFWwindow*, double, double);
	static void mC(GLFWwindow*, int, int, int);
	static void mPC(GLFWwindow*, double, double);
	static void wRC(GLFWwindow*, int, int);
};

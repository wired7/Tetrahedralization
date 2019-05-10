#pragma once
#include "FPSCameraController.h"
#include "ClusteringStageContext.h"
#include "GeometryRenderingController.h"

class ClusteringStageContext;
struct GLFWwindow;

class ClusteringStageController : public FPSCameraController<ClusteringStageController, ClusteringStageContext>,
								  public GeometryRenderingController<ClusteringStageController, ClusteringStageContext>
{
public:
	ClusteringStageController();
	~ClusteringStageController();
	static void kC(GLFWwindow*, int, int, int, int);
	static void sC(GLFWwindow*, double, double);
	static void mC(GLFWwindow*, int, int, int);
	static void mPC(GLFWwindow*, double, double);
	static void wRC(GLFWwindow*, int, int);
};

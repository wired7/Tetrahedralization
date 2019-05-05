#pragma once
#include "Controller.h"
#include "ClusteringStageContext.h"

class ClusteringStageContext;
struct GLFWwindow;

class ClusteringStageController : public Controller<ClusteringStageController, ClusteringStageContext>
{
public:
	bool surfaceRendering = true;

	bool firstMouse = false;
	float lastX = 0;
	float lastY = 0;

	ClusteringStageController();
	~ClusteringStageController();

	static void kC(GLFWwindow*, int, int, int, int);
	static void sC(GLFWwindow*, double, double);
	static void mC(GLFWwindow*, int, int, int);
	static void mPC(GLFWwindow*, double, double);
	static void wRC(GLFWwindow*, int, int);
};

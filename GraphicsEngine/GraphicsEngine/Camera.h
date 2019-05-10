#pragma once
#include "glew.h"
#include "glm.hpp"

const glm::vec3 FRONT(0, 0, 1);
const glm::vec3 BACK(0, 0, -1);
const glm::vec3 LEFT(-1, 0, 0);
const glm::vec3 RIGHT(1, 0, 0);
const glm::vec3 UP(0, 1, 0);
const glm::vec3 DOWN(0, -1, 0);

class Camera
{
private:
	int screenWidth;
	int screenHeight;
public:
	static Camera* activeCamera;
	static void setCamera(Camera*);
	glm::vec2 relativePosition;
	glm::vec2 relativeDimensions;
	glm::mat4 Projection;
	glm::mat4 OrthoProjection;
	glm::mat4 View;
	glm::vec3 lookAtVector;
	glm::vec3 camPosVector;
	glm::vec3 upVector;
	bool dirty = true;

	Camera(glm::vec2 relativePosition, glm::vec2 relativeDimensions, glm::vec3 pos, glm::vec3 lookAt, glm::vec3 up, glm::mat4 Projection);
	virtual ~Camera() {};
	virtual void update() = 0;
	virtual bool timeStepUpdate() { return false; };
	void setViewport(int width, int height);
	int getScreenWidth();
	int getScreenHeight();
};

class SphericalCamera : public Camera
{
private:
	double maxCamPhi;

public:
	double camTheta = 0;
	double camPhi = 0;
	GLfloat distance;

	SphericalCamera(glm::vec2 relativePosition, glm::vec2 relativeDimensions, glm::vec3 pos, glm::vec3 lookAt,
		glm::vec3 up, glm::mat4 Projection);
	virtual void update();
	virtual void translate(glm::vec2 offset);
};

class FPSCamera : public Camera
{
private:
	// Calculates the front vector from the Camera's (updated) Euler Angles
	const float YAW = -90.0f;
	const float PITCH = 0.0f;
	const float ZOOM = 45.0f;
	const float ROTATE_SENSITIVITY = 0.05f;
	const float SPEED_FAST = 0.1f;
	const float SPEED_SLOW = 0.01f;
	const float SPEED = SPEED_FAST;

public:
	bool freeRotation;
	float yaw;
	float pitch;
	// Camera options
	float timeStep;
	float zoom;
	glm::vec3 rightVector;
	glm::vec3 velocity;

	FPSCamera(glm::vec2 relativePosition, glm::vec2 relativeDimensions, glm::vec3 pos, glm::vec3 lookAt, glm::vec3 up, glm::mat4 Projection);	
	void changeSpeed();
	void moveInDirection(glm::vec3 direction);
	void rotate(float xoffset, float yoffset, GLboolean constrainPitch = true);
	void updateRotation();
	void update() override;
	void addVelocity(glm::vec3 vel);
	bool timeStepUpdate() override;
};
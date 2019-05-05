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

	Camera(glm::vec2 relativePosition, glm::vec2 relativeDimensions, glm::vec3 pos, glm::vec3 lookAt, glm::vec3 up, glm::mat4 Projection);
	virtual ~Camera() {};
	virtual void update() = 0;
	void setViewport();

	int getScreenWidth();
	int getScreenHeight();

private:
	int screenWidth;
	int screenHeight;
};

class SphericalCamera : public Camera
{
public:
	double camTheta = 0;
	double camPhi = 0;
	GLfloat distance;

	SphericalCamera(glm::vec2 relativePosition, glm::vec2 relativeDimensions, glm::vec3 pos, glm::vec3 lookAt,
		glm::vec3 up, glm::mat4 Projection);
	virtual void update();
	virtual void translate(glm::vec2 offset);

private:
	double maxCamPhi;
};

class FPSCamera : public Camera
{
public:
	glm::vec3 velocity;

	FPSCamera(glm::vec2 relativePosition, glm::vec2 relativeDimensions, glm::vec3 pos, glm::vec3 lookAt, glm::vec3 up, glm::mat4 Projection);
	void changeSpeed();

	void processKeyInput(glm::vec3 direction);
	void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
	void update();

	bool usingCamera;

private:
	// Calculates the front vector from the Camera's (updated) Eular Angles

	const float YAW = -90.0f;
	const float PITCH = 0.0f;
	const float SENSITIVTY = 0.05f;
	const float ZOOM = 45.0f;
	const float SPEED_FAST = 0.1f;
	const float SPEED_SLOW = 0.01f;
	const float SPEED = SPEED_FAST;

	float Yaw;
	float Pitch;
	// Camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	glm::vec3 Front;
	glm::vec3 Right;
	glm::vec3 Up;
};
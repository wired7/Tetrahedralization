#pragma once
#include "Camera.h"
#include "WindowContext.h"
#include <gtc/matrix_transform.hpp>

Camera* Camera::activeCamera = NULL;

Camera::Camera(glm::vec2 relativePosition, glm::vec2 relativeDimensions, glm::vec3 pos, glm::vec3 lookAt, glm::vec3 up, glm::mat4 Projection)
{
	this->relativePosition = relativePosition;
	this->relativeDimensions = relativeDimensions;

	auto widthHeight = WindowContext::context->getSize();
	screenWidth = widthHeight.first;
	screenHeight = widthHeight.second;

	glm::mat4 ratioMatrix(1);
	ratioMatrix[1][1] *= relativeDimensions.x;
	Projection = Projection * ratioMatrix;
	OrthoProjection = glm::ortho(0.0f, (float)1200, 0.0f, (float)800);
	this->Projection = Projection;

	View = glm::lookAt(pos, lookAt, up);
	camPosVector = pos;
	lookAtVector = lookAt;
	upVector = up;
};

void Camera::setViewport()
{
	auto widthHeight = WindowContext::context->getSize();
	screenWidth = widthHeight.first;
	screenHeight = widthHeight.second;
	glViewport(relativePosition.x * screenWidth, relativePosition.y * screenHeight, relativeDimensions.x * screenWidth,
		relativeDimensions.y * screenHeight);
};

void Camera::setCamera(Camera* cam)
{
	activeCamera = cam;
	activeCamera->setViewport();
};

int Camera::getScreenWidth()
{
	return WindowContext::context->getSize().first;
};

int Camera::getScreenHeight()
{
	return WindowContext::context->getSize().second;
};

SphericalCamera::SphericalCamera(glm::vec2 relativePosition, glm::vec2 relativeDimensions, glm::vec3 pos, glm::vec3 lookAt,
	glm::vec3 up, glm::mat4 Projection) :
	Camera(relativePosition, relativeDimensions, pos, lookAt, up, Projection)
{
	distance = length(camPosVector - lookAtVector);
	maxCamPhi = 0.7;

	camTheta = atan2(lookAt.z - pos.z, lookAt.x - pos.x);

	update();
};

void SphericalCamera::update()
{
	if (camPhi > maxCamPhi)
		camPhi = maxCamPhi;
	else if (camPhi < maxCamPhi * -1)
		camPhi = maxCamPhi * -1;

	lookAtVector = camPosVector + glm::vec3(cos(camTheta) * cos(camPhi), sin(camPhi), sin(camTheta) * cos(camPhi));

	View = lookAt(camPosVector, lookAtVector, upVector);

	auto widthHeight = WindowContext::context->getSize();
	int width = widthHeight.first;
	int height = widthHeight.second;

	Projection = glm::perspective(45.0f, (float)width / height, 0.1f, 1000.0f);
};

void SphericalCamera::translate(glm::vec2 offset)
{
	glm::vec3 diff(cos(camTheta), 0, sin(camTheta));
	camPosVector += diff * glm::vec3(offset.x, 0, offset.y);
};



FPSCamera::FPSCamera(glm::vec2 relativePosition, glm::vec2 relativeDimensions, glm::vec3 pos, glm::vec3 lookAt,
	glm::vec3 up, glm::mat4 Projection)
	: Camera(relativePosition, relativeDimensions, pos, lookAt, up, Projection)
{

	Yaw = YAW;
	Pitch = PITCH;
	MovementSpeed = SPEED_FAST;
	usingCamera = false;
	MouseSensitivity = SENSITIVTY;
	Up = up;
	camPosVector = pos;
	lookAtVector = lookAt;
};

void FPSCamera::changeSpeed() {
	if (MovementSpeed == SPEED_FAST)
		MovementSpeed = SPEED_SLOW;
	else
		MovementSpeed = SPEED_FAST;
};

void FPSCamera::processKeyInput(glm::vec3 direction) {

	float velocity = -MovementSpeed;// *direction;

	if (direction.z == 1)
		camPosVector -= lookAtVector * velocity;
	if (direction.z == -1)
		camPosVector += lookAtVector * velocity;
	if (direction.x == -1)
		camPosVector += Right * velocity;
	if (direction.x == 1)
		camPosVector -= Right * velocity;
	if (direction.y == 1)
		camPosVector -= UP * velocity;
	if (direction.y == -1)
		camPosVector += UP * velocity;

	View = glm::lookAt(camPosVector, camPosVector + lookAtVector, upVector);

};

void FPSCamera::processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	Yaw += xoffset;
	Pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;
	}

	// Update Front, Right and Up Vectors using the updated Eular angles
	update();
};

void FPSCamera::update()
{
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);
	// Also re-calculate the Right and Up vector
	Right = glm::normalize(glm::cross(Front, glm::vec3(0, 1, 0)));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	Up = glm::normalize(glm::cross(Right, Front));

	lookAtVector = Front;

	View = glm::lookAt(camPosVector, camPosVector + lookAtVector, upVector);
};
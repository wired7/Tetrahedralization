#pragma once
#include "Camera.h"
#include "WindowContext.h"
#include <gtc/matrix_transform.hpp>

Camera* Camera::activeCamera = NULL;

Camera::Camera(glm::vec2 relativePosition, glm::vec2 relativeDimensions, glm::vec3 pos, glm::vec3 lookAt, glm::vec3 up, glm::mat4 Projection) :
	relativePosition(relativePosition), relativeDimensions(relativeDimensions)
{
	auto widthHeight = WindowContext::context->getSize();
	screenWidth = widthHeight.first;
	screenHeight = widthHeight.second;

	setViewport(screenWidth, screenHeight);

	OrthoProjection = glm::ortho(0.0f, (float)screenWidth, 0.0f, 1000.0f);

	View = glm::lookAt(pos, lookAt, up);
	camPosVector = pos;
	lookAtVector = lookAt;
	upVector = up;
};

void Camera::setViewport(int width, int height)
{
	screenWidth = width;
	screenHeight = height;

	glViewport(relativePosition.x * screenWidth,
			   screenHeight - (relativePosition.y + relativeDimensions.y) * screenHeight,
			   screenWidth * relativeDimensions.x,
			   screenHeight * relativeDimensions.y);

	Projection = glm::perspective(45.0f, (float)width / height, 0.1f, 1000.0f);
};

void Camera::setCamera(Camera* cam)
{
	activeCamera = cam;
	auto widthHeight = WindowContext::context->getSize();
	int screenWidth = widthHeight.first;
	int screenHeight = widthHeight.second;
	activeCamera->setViewport(screenWidth, screenHeight);
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
	yaw = YAW;
	pitch = PITCH;
	timeStep = SPEED_FAST;
	freeRotation = false;
};

void FPSCamera::changeSpeed()
{
	if (timeStep == SPEED_FAST)
	{
		timeStep = SPEED_SLOW;
	}
	else
	{
		timeStep = SPEED_FAST;
	}
};

void FPSCamera::moveInDirection(glm::vec3 direction)
{
	camPosVector += timeStep * (lookAtVector * direction.z + rightVector * direction.x + upVector * direction.y);
	dirty = true;
};

void FPSCamera::rotate(float xoffset, float yoffset, GLboolean constrainPitch) {
	xoffset *= ROTATE_SENSITIVITY;
	yoffset *= ROTATE_SENSITIVITY;

	yaw += xoffset;
	pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
	}

	dirty = true;
};

void FPSCamera::updateRotation()
{
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	lookAtVector = glm::normalize(front);
	// Also re-calculate the Right and Up vector
	// Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	rightVector = glm::normalize(glm::cross(lookAtVector, glm::vec3(0, 1, 0)));
	upVector = glm::normalize(glm::cross(rightVector, lookAtVector));
};

void FPSCamera::update()
{
	View = glm::lookAt(camPosVector, camPosVector + lookAtVector, upVector);
}

void FPSCamera::addVelocity(glm::vec3 vel)
{
	velocity += vel;
	dirty = true;
}

bool FPSCamera::timeStepUpdate()
{
	if (dirty)
	{
		moveInDirection(velocity);
		updateRotation();
		update();
		if(glm::length(velocity) == 0)
			dirty = false;
		return true;
	}

	return false;
}
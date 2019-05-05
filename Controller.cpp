#pragma once
#include "Controller.h"

void(*AbstractController::key_callback)(GLFWwindow*, int, int, int, int) = nullptr;
void(*AbstractController::scroll_callback)(GLFWwindow*, double, double) = nullptr;
void(*AbstractController::mouse_callback)(GLFWwindow*, int, int, int) = nullptr;
void(*AbstractController::mousePos_callback)(GLFWwindow*, double, double) = nullptr;
void(*AbstractController::windowResize_callback)(GLFWwindow*, int, int) = nullptr;
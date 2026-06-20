#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

void initializeCamera(float viewportWidth, float viewportHeight);
void handleCameraMouse(GLFWwindow* window, double xpos, double ypos);
void processCameraInput(GLFWwindow* window, float deltaTime);
glm::mat4 getCameraViewMatrix();

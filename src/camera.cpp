#include "engine/camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace {
    glm::vec3 cameraPos(0.0f, 0.35f, 4.0f);
    glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

    float yaw = -90.0f;
    float pitch = 0.0f;
    float lastMouseX = 0.0f;
    float lastMouseY = 0.0f;
    bool firstMouse = true;
}

void initializeCamera(float viewportWidth, float viewportHeight) {
    lastMouseX = viewportWidth * 0.5f;
    lastMouseY = viewportHeight * 0.5f;
    firstMouse = true;
}

void handleCameraMouse(GLFWwindow* window, double xpos, double ypos) {
    (void)window;

    if (firstMouse) {
        lastMouseX = static_cast<float>(xpos);
        lastMouseY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastMouseX;
    float yoffset = lastMouseY - static_cast<float>(ypos);
    lastMouseX = static_cast<float>(xpos);
    lastMouseY = static_cast<float>(ypos);

    const float sensitivity = 0.12f;
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;
    pitch = glm::clamp(pitch, -75.0f, 75.0f);

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
    
    // Debug
    static int frameCount = 0;
    if (frameCount++ % 30 == 0) {
        std::cout << "Yaw: " << yaw << ", Pitch: " << pitch << "\n";
    }
}

void processCameraInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    const float moveSpeed = 2.0f * deltaTime;
    glm::vec3 flatFront(cameraFront.x, 0.0f, cameraFront.z);
    if (glm::length(flatFront) > 0.001f) {
        flatFront = glm::normalize(flatFront);
    }

    glm::vec3 flatRight = glm::normalize(glm::cross(flatFront, cameraUp));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += flatFront * moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= flatFront * moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= flatRight * moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += flatRight * moveSpeed;

    cameraPos.x = glm::clamp(cameraPos.x, -14.5f, 14.5f);
    cameraPos.z = glm::clamp(cameraPos.z, -14.5f, 14.5f);
    cameraPos.y = 0.35f;
}

glm::mat4 getCameraViewMatrix() {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

glm::vec3 getCameraPosition() {
    return cameraPos;
}

glm::vec3 getCameraFront() {
    return cameraFront;
}

glm::vec3 getCameraRight() {
    return glm::normalize(glm::cross(cameraFront, cameraUp));
}

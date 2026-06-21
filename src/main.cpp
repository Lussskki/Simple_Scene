#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/mesh.h"
#include "engine/model.h"

#include <iostream>

// Window size
const unsigned int WIDTH = 1000;
const unsigned int HEIGHT = 800;

// Called when the window is resized.
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}

// Prints GLFW errors in the terminal.
void glfw_error_callback(int error, const char* description) {
    (void)error;
    std::cerr << "GLFW Error: " << description << std::endl;
}

int main() {
    // 1) Window and OpenGL setup
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Textured OBJ Viewer", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    initializeCamera(static_cast<float>(WIDTH), static_cast<float>(HEIGHT));
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, handleCameraMouse);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // 2) Load CPU-side OBJ models.
    LoadedModel cottage = loadObjModel("assets/Cottage/Cottage_FREE.obj", "assets/Cottage/");
    if (cottage.indices.empty()) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    LoadedModel flashlight = loadObjModel("assets/Flashlight/Linterna.obj", "assets/Flashlight/");
    if (flashlight.indices.empty()) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // 3) Create GPU meshes from loaded models and generated shapes.
    Mesh cottageMesh = createMesh(cottage.vertices, cottage.indices);
    Mesh flashlightMesh = createMesh(flashlight.vertices, flashlight.indices);
    Mesh sphereMesh = createSphereMesh(0.5f, 32, 16);
    Mesh groundMesh = createGroundMesh();
    Mesh wallMesh = createWallMesh(6.0f, 3.0f, 0.0f);

    // 4) Load or create textures for every object.
    GLuint cottageTexture = loadTextureFromFile("assets/Cottage/Cottage_Clean/Cottage_Clean_Base_Color.png");
    if (!cottageTexture) {
        destroyMesh(cottageMesh);
        destroyMesh(flashlightMesh);
        destroyMesh(sphereMesh);
        destroyMesh(groundMesh);
        destroyMesh(wallMesh);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    GLuint flashlightTexture = loadTextureFromFile("assets/Flashlight/Texture/FlashlightTexture.jpg");
    if (!flashlightTexture) {
        destroyMesh(cottageMesh);
        destroyMesh(flashlightMesh);
        destroyMesh(sphereMesh);
        destroyMesh(groundMesh);
        destroyMesh(wallMesh);
        glDeleteTextures(1, &cottageTexture);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    GLuint groundTexture = createSolidColorTexture(70, 125, 58, 255);
    GLuint sunTexture = createSolidColorTexture(255, 220, 60, 255);
    GLuint wallTexture = createSolidColorTexture(20, 20, 20, 255);

    // 5) Load shaders from files.
    GLuint shaderProgram = createShaderProgram(
        "src/shaders/scene_shader.vert",
        "src/shaders/scene_shader.frag"
    );

    // 6) Static scene transforms. These do not change every frame.
    glm::mat4 cottageModel = glm::mat4(1.0f);
    cottageModel = glm::scale(cottageModel, glm::vec3(cottage.scale));
    cottageModel = glm::translate(cottageModel, glm::vec3(-cottage.center.x, -cottage.minBounds.y, -cottage.center.z));

    glm::mat4 wallModel = glm::mat4(1.0f);
    wallModel = glm::translate(wallModel, glm::vec3(6.0f, 0.0f, 0.0f));

    glm::vec3 sunPosition(4.0f, 8.0f, 10.0f);
    glm::vec3 lightDirection = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - sunPosition);
    // hide  
    // glm::vec3 flashlightPosition(6.0f, 0.25f, -0.6f);
    // glm::vec3 flashlightDirection(0.0f, 0.0f, 1.0f);

    float lastFrame = 0.0f;

    // 7) Render loop. Everything visible is drawn here every frame.
    while (!glfwWindowShouldClose(window)) {
        // Time between frames, used by camera movement.
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processCameraInput(window, deltaTime);

        // Camera matrices.
        int framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        float aspect = framebufferHeight > 0 ? (float)framebufferWidth / framebufferHeight : (float)WIDTH / HEIGHT;

        glm::mat4 view = getCameraViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, 0.03f, 60.0f);

        glm::vec3 cameraPosition = getCameraPosition();
        glm::vec3 cameraFront = getCameraFront();
        glm::vec3 cameraRight = getCameraRight();
        glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));

        glm::vec3 flashlightPosition = cameraPosition + cameraFront * 0.45f + cameraRight * 0.18f - cameraUp * 0.12f;
        glm::vec3 flashlightDirection = cameraFront;

        // Clear the previous frame.
        glClearColor(0.55f, 0.72f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Values shared by all objects using this shader.
        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture_diffuse"), 0);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 0.96f, 0.86f);
        glUniform1f(glGetUniformLocation(shaderProgram, "ambientStrength"), 0.45f);
        glUniform3fv(glGetUniformLocation(shaderProgram, "spotPosition"), 1, glm::value_ptr(flashlightPosition));
        glUniform3fv(glGetUniformLocation(shaderProgram, "spotDirection"), 1, glm::value_ptr(flashlightDirection));
        glUniform3f(glGetUniformLocation(shaderProgram, "spotColor"), 1.0f, 0.92f, 0.65f);
        glUniform1f(glGetUniformLocation(shaderProgram, "spotCutoff"), glm::cos(glm::radians(12.5f)));
        glUniform1f(glGetUniformLocation(shaderProgram, "spotOuterCutoff"), glm::cos(glm::radians(22.0f)));

        // Draw ground.
        glm::mat4 groundModel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(groundModel));
        glBindTexture(GL_TEXTURE_2D, groundTexture);
        drawMesh(groundMesh);

        // Draw cottage.
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(cottageModel));
        glBindTexture(GL_TEXTURE_2D, cottageTexture);
        drawMesh(cottageMesh);

        // Draw flashlight model near the camera.
        glm::mat4 flashlightModel = glm::mat4(1.0f);
        glm::vec3 flashlightXAxis = -cameraFront;
        glm::vec3 flashlightZAxis = -cameraRight;
        glm::vec3 flashlightYAxis = glm::normalize(glm::cross(flashlightZAxis, flashlightXAxis));

        glm::mat4 flashlightRotation = glm::mat4(1.0f);
        flashlightRotation[0] = glm::vec4(flashlightXAxis, 0.0f);
        flashlightRotation[1] = glm::vec4(flashlightYAxis, 0.0f);
        flashlightRotation[2] = glm::vec4(flashlightZAxis, 0.0f);

        flashlightModel = glm::translate(flashlightModel, flashlightPosition);
        flashlightModel *= flashlightRotation;
        flashlightModel = glm::scale(flashlightModel, glm::vec3(flashlight.scale * 0.08f));
        flashlightModel = glm::translate(
            flashlightModel,
            glm::vec3(-flashlight.center.x, -flashlight.minBounds.y, -flashlight.center.z)
        );

        glUniform1f(glGetUniformLocation(shaderProgram, "ambientStrength"), 0.45f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(flashlightModel));
        glBindTexture(GL_TEXTURE_2D, flashlightTexture);
        drawMesh(flashlightMesh);

        // Draw dark wall.
        glUniform1f(glGetUniformLocation(shaderProgram, "ambientStrength"), 0.45f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(wallModel));
        glBindTexture(GL_TEXTURE_2D, wallTexture);
        drawMesh(wallMesh);

        // Draw sun sphere with full ambient so it looks bright.
        glm::mat4 sunModel = glm::mat4(1.0f);
        sunModel = glm::translate(sunModel, sunPosition);
        sunModel = glm::scale(sunModel, glm::vec3(1.0f, 1.0f, 1.0f));
        glUniform1f(glGetUniformLocation(shaderProgram, "ambientStrength"), 1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(sunModel));
        glBindTexture(GL_TEXTURE_2D, sunTexture);
        drawMesh(sphereMesh);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 8) Cleanup. Every created OpenGL resource is deleted here.
    destroyMesh(cottageMesh);
    destroyMesh(flashlightMesh);
    destroyMesh(groundMesh);
    destroyMesh(sphereMesh);
    destroyMesh(wallMesh);

    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &cottageTexture);
    glDeleteTextures(1, &flashlightTexture);
    glDeleteTextures(1, &groundTexture);
    glDeleteTextures(1, &sunTexture);
    glDeleteTextures(1, &wallTexture);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

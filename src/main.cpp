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

// Window dimensions
const unsigned int WIDTH = 1000;
const unsigned int HEIGHT = 800;

// GLFW framebuffer size callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// GLFW error callback
void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error: " << description << std::endl;
}

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Textured OBJ Viewer", nullptr, nullptr);
    if(!window){ glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    initializeCamera(static_cast<float>(WIDTH), static_cast<float>(HEIGHT));
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, handleCameraMouse);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ std::cerr<<"Failed GLAD\n"; return -1; }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    LoadedModel cottage = loadObjModel("assets/Cottage_FREE.obj", "assets/");
    if (cottage.indices.empty()) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    Mesh cottageMesh = createMesh(cottage.vertices, cottage.indices);
    Mesh sphereMesh = createSphereMesh(0.5f, 32, 16);
    Mesh groundMesh = createGroundMesh();

    GLuint cottageTexture = loadTextureFromFile("assets/Cottage_Clean/Cottage_Clean_Base_Color.png");
    if (!cottageTexture) {
        destroyMesh(cottageMesh);
        destroyMesh(sphereMesh);
        destroyMesh(groundMesh);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    GLuint groundTexture = createSolidColorTexture(70, 125, 58, 255);
    GLuint sunTexture = createSolidColorTexture(255, 220, 60, 255);

    // --- Load shaders from files ---
    GLuint shaderProgram = createShaderProgram(
        "src/shaders/home_shader.vert",
        "src/shaders/home_shader.frag"
    );

    // Model, View, Projection 
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::scale(modelMat, glm::vec3(cottage.scale));
    modelMat = glm::translate(modelMat, glm::vec3(-cottage.center.x, -cottage.minBounds.y, -cottage.center.z));

    glm::vec3 sunPosition(4.0f, 8.0f, 10.0f);
    glm::vec3 lightDirection = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - sunPosition);

    float lastFrame = 0.0f;

    // Render loop 
    while(!glfwWindowShouldClose(window)){
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processCameraInput(window, deltaTime);

        int framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        float aspect = framebufferHeight > 0 ? (float)framebufferWidth / framebufferHeight : (float)WIDTH / HEIGHT;

        glm::mat4 view = getCameraViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, 0.03f, 60.0f);

        glClearColor(0.55f,0.72f,0.92f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"view"),1,GL_FALSE,glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"projection"),1,GL_FALSE,glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(shaderProgram,"lightDirection"), 1, glm::value_ptr(lightDirection));
        glUniform3f(glGetUniformLocation(shaderProgram,"lightColor"), 1.0f, 0.96f, 0.86f);
        glUniform1f(glGetUniformLocation(shaderProgram,"ambientStrength"), 0.45f);

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(shaderProgram,"texture_diffuse"),0);

        glm::mat4 groundModel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"model"),1,GL_FALSE,glm::value_ptr(groundModel));
        glBindTexture(GL_TEXTURE_2D,groundTexture);
        drawMesh(groundMesh);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"model"),1,GL_FALSE,glm::value_ptr(modelMat));
        glBindTexture(GL_TEXTURE_2D,cottageTexture);
        drawMesh(cottageMesh);

        glm::mat4 sunModel = glm::mat4(1.0f);
        sunModel = glm::translate(sunModel, sunPosition);
        sunModel = glm::scale(sunModel, glm::vec3(1.0f, 1.0f, 1.0f));
        glUniform1f(glGetUniformLocation(shaderProgram,"ambientStrength"), 1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"model"),1,GL_FALSE,glm::value_ptr(sunModel));
        glBindTexture(GL_TEXTURE_2D,sunTexture);
        drawMesh(sphereMesh);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup ---
    destroyMesh(cottageMesh);
    destroyMesh(groundMesh);
    destroyMesh(sphereMesh);
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1,&cottageTexture);
    glDeleteTextures(1,&groundTexture);
    glDeleteTextures(1,&sunTexture);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

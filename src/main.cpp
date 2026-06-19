#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/mesh.h"
#include "engine/model.h"

#include <iostream>

// Window dimensions
const unsigned int WIDTH = 1000;
const unsigned int HEIGHT = 800;

glm::vec3 cameraPos(0.0f, 0.35f, 4.0f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;
float lastMouseX = WIDTH * 0.5f;
float lastMouseY = HEIGHT * 0.5f;
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
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
}

void processInput(GLFWwindow* window, float deltaTime) {
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
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
    Mesh groundMesh = createGroundMesh();

    GLuint cottageTexture = loadTextureFromFile("assets/Cottage_Clean/Cottage_Clean_Base_Color.png");
    if (!cottageTexture) {
        destroyMesh(cottageMesh);
        destroyMesh(groundMesh);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    GLuint groundTexture = createSolidColorTexture(70, 125, 58, 255);

    // --- Load shaders from files ---
    GLuint shaderProgram = createShaderProgram(
        "src/shaders/home_shader.vert",
        "src/shaders/home_shader.frag"
    );

    // Model, View, Projection 
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::scale(modelMat, glm::vec3(cottage.scale));
    modelMat = glm::translate(modelMat, glm::vec3(-cottage.center.x, -cottage.minBounds.y, -cottage.center.z));

    float lastFrame = 0.0f;

    // Render loop 
    while(!glfwWindowShouldClose(window)){
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window, deltaTime);

        int framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        float aspect = framebufferHeight > 0 ? (float)framebufferWidth / framebufferHeight : (float)WIDTH / HEIGHT;

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, 0.03f, 60.0f);

        glClearColor(0.55f,0.72f,0.92f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"view"),1,GL_FALSE,glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"projection"),1,GL_FALSE,glm::value_ptr(projection));

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(shaderProgram,"texture_diffuse"),0);

        glm::mat4 groundModel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"model"),1,GL_FALSE,glm::value_ptr(groundModel));
        glBindTexture(GL_TEXTURE_2D,groundTexture);
        drawMesh(groundMesh);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"model"),1,GL_FALSE,glm::value_ptr(modelMat));
        glBindTexture(GL_TEXTURE_2D,cottageTexture);
        drawMesh(cottageMesh);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup ---
    destroyMesh(cottageMesh);
    destroyMesh(groundMesh);
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1,&cottageTexture);
    glDeleteTextures(1,&groundTexture);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

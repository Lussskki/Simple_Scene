#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "tinyobjloader/tiny_obj_loader.h"

#include <iostream>
#include <vector>
#include <cfloat>
#include <algorithm>
#include <map>
#include <tuple>
#include <string>
#include <fstream>
#include <sstream>

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

// --- Helper to load shader files ---
std::string loadShaderSource(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filepath << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
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

    // --- Load OBJ, deduplicate vertices and fix UVs ---
    struct Vertex {
        glm::vec3 pos;
        glm::vec2 uv;
        bool operator<(const Vertex& other) const {
            return std::tie(pos.x,pos.y,pos.z,uv.x,uv.y) < std::tie(other.pos.x,other.pos.y,other.pos.z,other.uv.x,other.uv.y);
        }
    };

    std::vector<float> vertices; // x,y,z,u,v
    std::vector<unsigned int> indices;
    std::map<Vertex,unsigned int> uniqueVertices;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "assets/Cottage_FREE.obj", "assets/");
    if (!err.empty()) std::cerr << "TinyOBJLoader: " << err << std::endl;
    if (!ret) {
        std::cerr << "Failed to load OBJ!\n";
        return -1;
    }

    for (const auto& shape : shapes) {
        for (const auto& ind : shape.mesh.indices) {
            glm::vec3 position(
                attrib.vertices[3 * ind.vertex_index + 0],
                attrib.vertices[3 * ind.vertex_index + 1],
                attrib.vertices[3 * ind.vertex_index + 2]
            );

            glm::vec2 uv(0.0f, 0.0f);
            if (ind.texcoord_index >= 0) {
                uv.x = attrib.texcoords[2 * ind.texcoord_index + 0];
                uv.y = 1.0f - attrib.texcoords[2 * ind.texcoord_index + 1];
            }

            Vertex vert{position, uv};
            if (uniqueVertices.count(vert) == 0) {
                uniqueVertices[vert] = static_cast<unsigned int>(vertices.size() / 5);
                vertices.push_back(position.x);
                vertices.push_back(position.y);
                vertices.push_back(position.z);
                vertices.push_back(uv.x);
                vertices.push_back(uv.y);
            }

            indices.push_back(uniqueVertices[vert]);
        }
    }

    std::cout << "Loaded vertices: " << vertices.size()/5 << ", indices: " << indices.size() << "\n";

    // --- Compute center & scale ---
    glm::vec3 minV(FLT_MAX), maxV(-FLT_MAX);
    for(size_t i=0;i<vertices.size();i+=5){
        minV.x = std::min(minV.x, vertices[i+0]);
        minV.y = std::min(minV.y, vertices[i+1]);
        minV.z = std::min(minV.z, vertices[i+2]);
        maxV.x = std::max(maxV.x, vertices[i+0]);
        maxV.y = std::max(maxV.y, vertices[i+1]);
        maxV.z = std::max(maxV.z, vertices[i+2]);
    }

    glm::vec3 center = (minV + maxV) * 0.5f;
    glm::vec3 diff = maxV - minV;
    float maxDim = std::max({diff.x,diff.y,diff.z});
    float scale = 2.0f / maxDim;

    // --- VAO / VBO / EBO ---
    GLuint VAO,VBO,EBO;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glGenBuffers(1,&EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(float),vertices.data(),GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size()*sizeof(unsigned int),indices.data(),GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    float groundVertices[] = {
        -15.0f, -0.01f, -15.0f, 0.0f, 0.0f,
         15.0f, -0.01f, -15.0f, 8.0f, 0.0f,
         15.0f, -0.01f,  15.0f, 8.0f, 8.0f,
        -15.0f, -0.01f,  15.0f, 0.0f, 8.0f
    };
    unsigned int groundIndices[] = { 0, 1, 2, 2, 3, 0 };

    GLuint groundVAO, groundVBO, groundEBO;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);
    glGenBuffers(1, &groundEBO);

    glBindVertexArray(groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // --- Load texture ---
    int texW, texH, texC;
    unsigned char* data = stbi_load("assets/Cottage_Clean/Cottage_Clean_Base_Color.png",&texW,&texH,&texC,0);
    if(!data){ std::cerr<<"Failed to load texture\n"; return -1; }
    GLuint texture;
    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D,texture);
    glTexImage2D(GL_TEXTURE_2D,0,(texC==4?GL_RGBA:GL_RGB),texW,texH,0,(texC==4?GL_RGBA:GL_RGB),GL_UNSIGNED_BYTE,data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    unsigned char groundPixel[] = { 70, 125, 58, 255 };
    GLuint groundTexture;
    glGenTextures(1, &groundTexture);
    glBindTexture(GL_TEXTURE_2D, groundTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, groundPixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // --- Load shaders from files ---
    std::string vertexShaderSrc   = loadShaderSource("src/shaders/home_shader.vert");
    std::string fragmentShaderSrc = loadShaderSource("src/shaders/home_shader.frag");

    auto compileShader = [](GLenum type,const char* src){
        GLuint s = glCreateShader(type);
        glShaderSource(s,1,&src,nullptr);
        glCompileShader(s);
        int success; char info[512];
        glGetShaderiv(s,GL_COMPILE_STATUS,&success);
        if(!success){ glGetShaderInfoLog(s,512,NULL,info); std::cerr<<"Shader compile error: "<<info<<"\n"; }
        return s;
    };

    GLuint vertexShader   = compileShader(GL_VERTEX_SHADER, vertexShaderSrc.c_str());
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc.c_str());

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram,vertexShader);
    glAttachShader(shaderProgram,fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Model, View, Projection 
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::scale(modelMat, glm::vec3(scale));
    modelMat = glm::translate(modelMat, glm::vec3(-center.x, -minV.y, -center.z));

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
        glBindVertexArray(groundVAO);
        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"model"),1,GL_FALSE,glm::value_ptr(modelMat));
        glBindTexture(GL_TEXTURE_2D,texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES,indices.size(),GL_UNSIGNED_INT,0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup ---
    glDeleteVertexArrays(1,&VAO);
    glDeleteVertexArrays(1,&groundVAO);
    glDeleteBuffers(1,&VBO);
    glDeleteBuffers(1,&groundVBO);
    glDeleteBuffers(1,&EBO);
    glDeleteBuffers(1,&groundEBO);
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1,&texture);
    glDeleteTextures(1,&groundTexture);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

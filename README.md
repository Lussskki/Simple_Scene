# Simple Scene

A small OpenGL scene that loads a textured cottage OBJ model, draws a simple ground surface, and uses a first-person style camera.

## Project Structure

```text
assets/                 Model and texture assets
include/                Third-party headers and project headers
include/engine/         Project engine-style headers
lib/                    Linked libraries
src/                    Project source files
src/shaders/            GLSL shader files
```

Main project files:

```text
src/main.cpp            Window setup, camera input, scene setup, render loop
src/shader.cpp          Shader file loading, shader compilation, shader program linking
src/texture.cpp         Image texture loading and solid color texture creation
src/mesh.cpp            VAO/VBO/EBO creation, mesh drawing, mesh cleanup, ground mesh creation
src/model.cpp           OBJ loading, vertex/index extraction, bounds/center/scale calculation
```

Headers:

```text
include/engine/shader.h
include/engine/texture.h
include/engine/mesh.h
include/engine/model.h
```

## Build

This project is configured for the local C++ CLI through:

```text
.cpp-cli-config.json
```

The CLI compiler is set to:

```text
C:\msys64\ucrt64\bin\g++.exe
```

Use the C++ CLI build action normally. It should compile the sources in `src/`, include `src/glad.c`, link GLFW/OpenGL, and output:

```text
scene.exe
```

Equivalent manual command:

```bat
C:\msys64\ucrt64\bin\g++.exe -std=c++17 src\glad.c src\main.cpp src\mesh.cpp src\model.cpp src\shader.cpp src\texture.cpp src\tinyobjloader_impl.cpp -Iinclude -Llib -lglfw3 -lgdi32 -lopengl32 -o scene.exe
```

VS Code build settings are also stored in:

```text
.vscode/tasks.json
```

## Run

After building:

```bash
.\scene.exe
```

## Controls

```text
W / A / S / D   Move camera
Mouse           Look around
Esc             Close window
```

## Current Scene Flow

The scene is created in `src/main.cpp`:

1. Initialize GLFW and GLAD.
2. Enable depth testing.
3. Load the cottage OBJ model with `loadObjModel`.
4. Create GPU mesh data with `createMesh`.
5. Create the ground surface with `createGroundMesh`.
6. Load the cottage texture with `loadTextureFromFile`.
7. Create the ground texture with `createSolidColorTexture`.
8. Create the shader program with `createShaderProgram`.
9. Build model, view, and projection matrices.
10. Draw the ground mesh.
11. Draw the cottage mesh.
12. Clean up meshes, textures, shader program, and the GLFW window.

## Adding A New Model

A model is made from:

```text
OBJ file
Texture file
Mesh
Model matrix
Draw call
Cleanup
```

Example:

```cpp
LoadedModel tree = loadObjModel("assets/tree.obj", "assets/");
Mesh treeMesh = createMesh(tree.vertices, tree.indices);
GLuint treeTexture = loadTextureFromFile("assets/tree_texture.png");
```

Create a transform:

```cpp
glm::mat4 treeModel = glm::mat4(1.0f);
treeModel = glm::translate(treeModel, glm::vec3(3.0f, 0.0f, -4.0f));
treeModel = glm::scale(treeModel, glm::vec3(tree.scale));
```

Draw it inside the render loop:

```cpp
glUniformMatrix4fv(
    glGetUniformLocation(shaderProgram, "model"),
    1,
    GL_FALSE,
    glm::value_ptr(treeModel)
);
glBindTexture(GL_TEXTURE_2D, treeTexture);
drawMesh(treeMesh);
```

Clean it up before exit:

```cpp
destroyMesh(treeMesh);
glDeleteTextures(1, &treeTexture);
```

## Notes

- Vertex layout is currently `x, y, z, u, v`.
- The shader uses one diffuse texture sampler named `texture_diffuse`.
- `model.cpp` calculates model bounds, center, and scale after loading OBJ data.
- `mesh.cpp` assumes each vertex has 5 floats: 3 for position and 2 for UV.

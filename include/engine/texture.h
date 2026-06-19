#pragma once

#include <glad/glad.h>

GLuint loadTextureFromFile(const char* path);
GLuint createSolidColorTexture(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

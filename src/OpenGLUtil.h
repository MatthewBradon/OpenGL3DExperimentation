# pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <optional>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint createShader(std::string filePath, GLenum shaderType)  {

    std::ifstream triangleVertFile(filePath, std::ios::in | std::ios::binary);

    if(!triangleVertFile) {
        std::cerr << "Failed to open " << filePath << std::endl;
    }

    std::string shaderFileString((std::istreambuf_iterator<char>(triangleVertFile)),
                         std::istreambuf_iterator<char>());

    const char * shaderSource = shaderFileString.c_str();
    
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    // Checking for success
    {
        int success;
        char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if(!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "shader COMPILATION FAILED\n" << infoLog << std::endl;
        }
    }   

    return shader;
}

int createTexture(const std::string& filePath, GLuint& textureID) {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int image_width, image_height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    auto path = std::filesystem::absolute(filePath);
    unsigned char *image_data = stbi_load(path.string().c_str(), &image_width, &image_height, &nrChannels, 0);

    if (!image_data) {
        std::cerr << "Failed to load texture: " << stbi_failure_reason() << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "Image dimensions: " << image_width << "x" << image_height << std::endl;


    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, image_width, image_height, 0, format, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image_data);
    return 0;
}


void checkGLError(const std::string& msg) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error after " << msg << ": " << std::hex << err << std::endl;
    }
}
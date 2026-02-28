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

int createTexture(const char *filePath, bool flipVertically=true) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    int image_width, image_height, nrChannels;
    stbi_set_flip_vertically_on_load(flipVertically);
    auto path = std::filesystem::absolute(filePath);
    std::cout << "Loading texture: " << path.string() << std::endl;
    unsigned char *image_data = stbi_load(path.string().c_str(), &image_width, &image_height, &nrChannels, 0);

    if (!image_data) {
        std::cerr << "Failed to load texture: " << stbi_failure_reason() << std::endl;
        stbi_image_free(image_data);
        glfwTerminate();
        return 0;
    }

    std::cout << "Image dimensions: " << image_width << "x" << image_height << std::endl;


    GLenum format;
    if(nrChannels == 1)
        format = GL_RED;
    else if(nrChannels == 3)
        format = GL_RGB;
    else if(nrChannels == 4)
        format = GL_RGBA;
    else {
        std::cerr << "Unsupported number of channels: " << nrChannels << std::endl;
        stbi_image_free(image_data);
        glfwTerminate();
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, format, image_width, image_height, 0, format, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(image_data);
    return textureID;
}

int allocateTexture(GLenum internalFormat, int width, int height, GLenum format, GLenum type) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

int attachTextureToFramebuffer(GLuint framebuffer, GLuint texture, GLenum attachment) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);
    return 0;
}

int createRenderbuffer(GLenum internalFormat, int width, int height) {
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
    return rbo;
}


int createCubeMapTexture(std::vector<std::string>& textureFaces) {
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (unsigned int i = 0; i < textureFaces.size(); i++) {
        int image_width, image_height, nrChannels;
        stbi_set_flip_vertically_on_load(false);
        auto path = std::filesystem::absolute(textureFaces[i]);
        std::cout << "Loading cubemap face: " << path.string() << std::endl;
        unsigned char *image_data = stbi_load(path.string().c_str(), &image_width, &image_height, &nrChannels, 0);

        if (!image_data) {
            std::cerr << "Failed to load cubemap face: " << stbi_failure_reason() << std::endl;
            stbi_image_free(image_data);
            glfwTerminate();
            return 0;
        }

        GLenum format;
        if(nrChannels == 1)
            format = GL_RED;
        else if(nrChannels == 3)
            format = GL_RGB;
        else if(nrChannels == 4)
            format = GL_RGBA;
        else {
            std::cerr << "Unsupported number of channels in cubemap face: " << nrChannels << std::endl;
            stbi_image_free(image_data);
            glfwTerminate();
            return 0;
        }

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, image_width, image_height, 0, format, GL_UNSIGNED_BYTE, image_data);
        
        
        stbi_image_free(image_data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


void checkGLError(const std::string& msg) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error after " << msg << ": " << std::hex << err << std::endl;
    }
}
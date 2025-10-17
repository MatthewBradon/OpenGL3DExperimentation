# pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <optional>


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
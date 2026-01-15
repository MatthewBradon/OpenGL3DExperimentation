#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "OpenGLUtil.h"

class Shader {
    public:
        GLuint ID;

        Shader(const char* vertexPath, const char* fragmentPath) {
            GLuint vertexShader = createShader(vertexPath, GL_VERTEX_SHADER);
            GLuint fragmentShader = createShader(fragmentPath, GL_FRAGMENT_SHADER);

            ID = glCreateProgram();
            glAttachShader(ID, vertexShader);
            glAttachShader(ID, fragmentShader);
            glLinkProgram(ID);
            
            // Check success
            {
                int success;
                char infoLog[512];
                glGetProgramiv(ID, GL_LINK_STATUS, &success);
                if(!success) {
                    glGetProgramInfoLog(ID, 512, NULL, infoLog);
                    std::cerr << "PROGRAM LINKING FAILED\n" << infoLog << std::endl;
                }
            }

            glDetachShader(ID, vertexShader);
            glDetachShader(ID, fragmentShader);

            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
        }

        void use() {
            glUseProgram(ID);
        }

        void setBool(const std::string &name, bool value) const {         
            glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
        }

        void setInt(const std::string &name, int value) const { 
            glUniform1i(glGetUniformLocation(ID,  name.c_str()), value); 
        }

        void setFloat(const std::string &name, float value) const { 
            glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
        }

        void setMat4(const std::string &name, const glm::mat4 &mat) const {
            glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }

        void setVec3(const std::string &name, const glm::vec3 &vec) const {
            glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &vec[0]);
        }

        void setVec3(const std::string &name, float x, float y, float z) const {
            glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
        }

        void setVec4(const std::string &name, const glm::vec4 &vec) const {
            glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &vec[0]);
        }

        void setVec4(const std::string &name, float x, float y, float z, float w) const {
            glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
        }

        void setMat3(const std::string &name, const glm::mat3 &mat) const {
            glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }

        void setMat2(const std::string &name, const glm::mat2 &mat) const {
            glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }
        
};

#endif // SHADER_H
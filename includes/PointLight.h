#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include <glm/glm.hpp>
#include "Cube.h"

// Forward-declare Shader to avoid include ordering/circular issues in headers
class Shader;

class PointLight {
public:

    int index;

    glm::vec3 position;
    
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    
    float constant;
    float linear;
    float quadratic;

    float aspect;
    float near;
    float far;

    std::vector<glm::mat4> shadowTransforms;

    PointLight(int index, glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic, float aspect, float near, float far)
        : index(index), position(position), ambient(ambient), diffuse(diffuse), specular(specular), constant(constant), linear(linear), quadratic(quadratic), aspect(aspect), near(near), far(far), __debugCube(std::vector<Texture>()) {

            glm::mat4 shadowProjection = glm::perspective(glm::radians(90.0f), aspect, near, far);

            shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
            shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
            shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
            shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
            shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
            shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        }


    ~PointLight() {}

    void drawDebugCube(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {

        glm::mat4 model = glm::mat4(1.0f);

        shader.use();
        
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(0.1f));
        shader.setMat4("model", model);

        __debugCube.Draw(shader);
    }

    void setShaderUniforms(Shader& shader) {
        std::string baseName = "pointLights[" + std::to_string(index) + "]";
        shader.setVec3(baseName + ".position", position);
        shader.setVec3(baseName + ".ambient", ambient);
        shader.setVec3(baseName + ".diffuse", diffuse);
        shader.setVec3(baseName + ".specular", specular);
        shader.setFloat(baseName + ".constant", constant);
        shader.setFloat(baseName + ".linear", linear);
        shader.setFloat(baseName + ".quadratic", quadratic);
        shader.setFloat(baseName + ".far_plane", far);
        // Single global cubemap sampler is at texture unit 2
        shader.setInt("shadowCubeMap", 2);

    }

    void setDepthShaderUniforms(Shader& shader) {
        shader.setFloat("far_plane", far);
        shader.setVec3("lightPos", position);
        for (int i = 0; i < 6; i++) {
            shader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        }
    }

    // Update position and recompute the 6 shadow transform matrices for the cubemap
    void updatePosition(const glm::vec3& newPos) {
        position = newPos;
        shadowTransforms.clear();

        glm::mat4 shadowProjection = glm::perspective(glm::radians(90.0f), aspect, near, far);

        shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProjection * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    }

    void bindShadowMap(GLuint shadowCubemap) {
        // Bind single global shadow cubemap to texture unit 2
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubemap);
    }

private:
    Cube __debugCube;
};


#endif // POINTLIGHT_H
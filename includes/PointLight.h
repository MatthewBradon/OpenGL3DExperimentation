#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include <glm/glm.hpp>
#include "Cube.h"

// Forward-declare Shader to avoid include ordering/circular issues in headers
class Shader;

class PointLight {
public:
    glm::vec3 position;
    
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    
    float constant;
    float linear;
    float quadratic;

    PointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic)
        : position(position), ambient(ambient), diffuse(diffuse), specular(specular), constant(constant), linear(linear), quadratic(quadratic), __debugCube(std::vector<Texture>()) {
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

    void setShaderUniforms(Shader& shader, int index) {
        std::string baseName = "pointLights[" + std::to_string(index) + "]";
        shader.setVec3(baseName + ".position", position);
        shader.setVec3(baseName + ".ambient", ambient);
        shader.setVec3(baseName + ".diffuse", diffuse);
        shader.setVec3(baseName + ".specular", specular);
        shader.setFloat(baseName + ".constant", constant);
        shader.setFloat(baseName + ".linear", linear);
        shader.setFloat(baseName + ".quadratic", quadratic);
    }

private:
    Cube __debugCube;
};


#endif // POINTLIGHT_H
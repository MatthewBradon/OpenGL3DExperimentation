#ifndef PLANE_H
#define PLANE_H

#include <glm/glm.hpp>
#include "Mesh.h"

class Plane {
public:
    Mesh mesh;

    Plane(const glm::vec3& normal, float d, const std::vector<Texture>& textures, float size=100.0f) : normal(glm::normalize(normal)), d(d), mesh(Mesh()) {
        glm::vec3 u, v;
        if (fabs(normal.x) > fabs(normal.y)) {
            u = glm::normalize(glm::cross(normal, glm::vec3(0.0f, 1.0f, 0.0f)));
        } else {
            u = glm::normalize(glm::cross(normal, glm::vec3(1.0f, 0.0f, 0.0f)));
        }
        v = glm::normalize(glm::cross(normal, u));

        glm::vec3 center = normal * d;

        std::vector<Vertex> vertices(4);
        vertices[0].Position = center + (u + v) * size;
        vertices[1].Position = center + (u - v) * size;
        vertices[2].Position = center + (-u - v) * size;
        vertices[3].Position = center + (-u + v) * size;

        for (auto& vertex : vertices) {
            vertex.Normal = normal;
            vertex.TexCoords = glm::vec2(
                (vertex.Position.x / (2.0f * size)) + 0.5f,
                (vertex.Position.z / (2.0f * size)) + 0.5f
            );
        }

        std::vector<GLuint> indices = {0, 1, 2, 2, 3, 0};

        mesh = Mesh(vertices, indices, textures);
    }

    glm::vec3 getNormal() const {
        return normal;
    }
    float getD() const {
        return d;
    }

    void Draw(Shader &shader) {
        mesh.Draw(shader);
    }

private:
    glm::vec3 normal;
    float d;
};

#endif // PLANE_H
#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <Shader.h>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    Vertex() : Position(0.0f), Normal(0.0f), TexCoords(0.0f) {}

    Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texCoords)
        : Position(position), Normal(normal), TexCoords(texCoords) {}
};

struct Texture {
    GLuint id;
    std::string type;
    std::string path;

    Texture() : id(0), type(""), path("") {}

    Texture(GLuint textureID, const std::string& textureType, const std::string& texturePath)
        : id(textureID), type(textureType), path(texturePath) {}

};

class Mesh {
	public:
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		std::vector<Texture> textures;

        Mesh() {}

		Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures) {
            this->vertices = vertices;
            this->indices = indices;
            this->textures = textures;

            setupMesh();
        }

		void Draw(Shader &shader) {
            GLuint diffuseNr = 0;
            GLuint specularNr = 0;
            for (unsigned int i = 0; i < textures.size(); i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, textures[i].id);

                std::string number;
                std::string name = textures[i].type;
                if (name == "texture_diffuse") {
                    number = std::to_string(diffuseNr);
                    diffuseNr++;
                } else if (name == "texture_specular") {
                    number = std::to_string(specularNr);
                    specularNr++;
                }

                shader.setInt(("material."+ name + "[" + number + "]").c_str(), i);
                
            }

            shader.setInt("material.diffuseCount", diffuseNr);
            shader.setInt("material.specularCount", specularNr);
            shader.setFloat("material.shininess", 32.0f);

            glActiveTexture(GL_TEXTURE0);

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
	private:
		GLuint VAO, VBO, EBO;

		void setupMesh() {
            // Generate our buffers
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            // Bind our buffers
            glBindVertexArray(VAO);

            // Bind VBO put vertex data
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() *  sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

            // Bind EBO put indices data
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

            //   3       3       2
            //|-----|----------|---|
            // x y z  nx ny nz  u v

            // Layout 0 positions
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

            // Layout 1 normal
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
            
            // Layout 2 texture
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
            
            // Unbind the VAO
            glBindVertexArray(0);
        }
};

#endif // MESH_H
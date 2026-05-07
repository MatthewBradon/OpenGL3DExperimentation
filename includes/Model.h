#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"
#include <filesystem>
#include "Shader.h"
#include "OpenGLUtil.h"

class Model {
public:
    Model(std::string path) {
        loadModel(path);
    }

    void Draw(Shader &shader) {
        for (unsigned int i = 0; i < meshes.size(); i++) {
            meshes[i].Draw(shader);
        }
    }

private:
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> loadedTextures;

    static glm::mat4 aiMatToGlm(const aiMatrix4x4 &m) {
        glm::mat4 mat;
        mat[0][0] = m.a1; mat[1][0] = m.a2; mat[2][0] = m.a3; mat[3][0] = m.a4;
        mat[0][1] = m.b1; mat[1][1] = m.b2; mat[2][1] = m.b3; mat[3][1] = m.b4;
        mat[0][2] = m.c1; mat[1][2] = m.c2; mat[2][2] = m.c3; mat[3][2] = m.c4;
        mat[0][3] = m.d1; mat[1][3] = m.d2; mat[2][3] = m.d3; mat[3][3] = m.d4;
        return mat;
    }

    void loadModel(std::string path) {
        Assimp::Importer importer;
        // Do not flip UVs here; let texture loader handle image orientation.
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ERROR:ASSIMP::" << importer.GetErrorString() << std::endl;
            return;
        }

        directory = std::filesystem::path(path).parent_path().string();

        // Start recursive node processing with identity transform
        processNode(scene->mRootNode, scene, aiMatrix4x4());
    }

    void processNode(aiNode *node, const aiScene *scene, const aiMatrix4x4& parentTransform) {
        aiMatrix4x4 currentTransform = parentTransform * node->mTransformation;

        for(unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene, currentTransform));
        }

        for(unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene, currentTransform);
        }
    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene, const aiMatrix4x4& nodeTransform) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        glm::mat4 nodeMat = aiMatToGlm(nodeTransform);
        glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(nodeMat)));

        for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            glm::vec3 vecPos;

            // positions
            if(mesh->HasPositions()) {
                vecPos.x = mesh->mVertices[i].x;
                vecPos.y = mesh->mVertices[i].y;
                vecPos.z = mesh->mVertices[i].z;
                glm::vec4 pos = nodeMat * glm::vec4(vecPos, 1.0f);
                vertex.Position = glm::vec3(pos);
            } else {
                vertex.Position = glm::vec3(0.0f);
            }

            // normals
            if (mesh->HasNormals()) {
                glm::vec3 n;
                n.x = mesh->mNormals[i].x;
                n.y = mesh->mNormals[i].y;
                n.z = mesh->mNormals[i].z;
                vertex.Normal = glm::normalize(normalMat * n);
            } else {
                vertex.Normal = glm::vec3(0.0f);
            }

            // texture coordinates
            if(mesh->mTextureCoords[0]) {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;

                if (mesh->HasTangentsAndBitangents()) {
                    glm::vec3 t;
                    t.x = mesh->mTangents[i].x;
                    t.y = mesh->mTangents[i].y;
                    t.z = mesh->mTangents[i].z;
                    vertex.Tangent = glm::normalize(normalMat * t);

                    glm::vec3 b;
                    b.x = mesh->mBitangents[i].x;
                    b.y = mesh->mBitangents[i].y;
                    b.z = mesh->mBitangents[i].z;
                    vertex.Bitangent = glm::normalize(normalMat * b);
                } else {
                    vertex.Tangent = glm::vec3(0.0f);
                    vertex.Bitangent = glm::vec3(0.0f);
                }
            } else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
                vertex.Tangent = glm::vec3(0.0f);
                vertex.Bitangent = glm::vec3(0.0f);
            }

            vertices.push_back(vertex);
        }

        // process indices
        for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // process material
        if(mesh->mMaterialIndex >= 0) {
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

            // legacy diffuse
            std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

            // Some glTFs put color in emissive
            std::vector<Texture> emissiveMaps = loadMaterialTextures(material, aiTextureType_EMISSIVE, "texture_diffuse", scene);
            textures.insert(textures.end(), emissiveMaps.begin(), emissiveMaps.end());

            // normal maps
            std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal", scene);
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

            // height maps
            std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_height", scene);
            textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

            // Some files use DISPLACEMENT for height/displacement maps — load those too as height
            std::vector<Texture> displacementMaps = loadMaterialTextures(material, aiTextureType_DISPLACEMENT, "texture_height", scene);
            textures.insert(textures.end(), displacementMaps.begin(), displacementMaps.end());
        }

        return Mesh(vertices, indices, textures);
    }

    std::vector<Texture> loadMaterialTextures(aiMaterial *material, aiTextureType type, std::string typeName, const aiScene* scene) {
        std::vector<Texture> textures;

        for(unsigned int i = 0; i < material->GetTextureCount(type); i++) {
            aiString str;
            material->GetTexture(type, i, &str);

            // check if texture was loaded before
            bool skip = false;
            for(unsigned int j = 0; j < loadedTextures.size(); j++) {
                if (loadedTextures[j].path == std::string(str.C_Str())) {
                    textures.push_back(loadedTextures[j]);
                    skip = true;
                    break;
                }
            }

            Texture texture;
            if(!skip) {
                std::string texStr = std::string(str.C_Str());

                // Embedded texture ("*<index>")
                if (!texStr.empty() && texStr[0] == '*') {
                    int embedIndex = atoi(texStr.c_str() + 1);
                    if (scene && embedIndex >= 0 && embedIndex < (int)scene->mNumTextures && scene->mTextures[embedIndex]) {
                        aiTexture* atex = scene->mTextures[embedIndex];
                        if (atex->mHeight == 0) {
                            // compressed image (PNG/JPEG) in memory
                            int dataSize = atex->mWidth;
                            const unsigned char* data = reinterpret_cast<const unsigned char*>(atex->pcData);
                            texture.id = createTextureFromMemory(data, dataSize, false);
                        } else {
                            // raw RGBA data
                            int width = atex->mWidth;
                            int height = atex->mHeight;
                            const unsigned char* data = reinterpret_cast<const unsigned char*>(atex->pcData);
                            texture.id = createTextureFromRawRGBA(data, width, height, 4);
                        }
                    } else {
                        std::cerr << "Embedded texture index out of range: " << texStr << std::endl;
                        texture.id = 0;
                    }
                } else {
                    // External file
                    std::string filename = texStr;
                    std::string fullpath = directory + '/' + filename;
                    texture.id = createTexture(fullpath.c_str(), true);
                }

                texture.type = typeName;
                texture.path = std::string(str.C_Str());
                textures.push_back(texture);
                if (typeName == "texture_height") {
                    std::cout << "[Model] Loaded height map: " << texture.path << " id=" << texture.id << std::endl;
                }
                loadedTextures.push_back(texture);
            }
        }

        return textures;
    }

};

#endif // MODEL_H

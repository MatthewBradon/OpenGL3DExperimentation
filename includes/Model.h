

#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"
#include "OpenGLUtil.h"


class Model {
	public:
		Model(char *path) {
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


		void loadModel(std::string path) {
        
            Assimp::Importer importer;
            const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        
            if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cout << "ERROR:ASSIMP::" << importer.GetErrorString() << std::endl;
                return;
            }

            directory = path.substr(0, path.find_last_of('/'));

            processNode(scene->mRootNode, scene);
        }

        void processNode(aiNode *rootNode, const aiScene *scene) {

            std::vector<aiNode*> nodes;
            nodes.push_back(rootNode);

            while(!nodes.empty()) {
                aiNode *currentNode = nodes.back();
                nodes.pop_back();

                for(unsigned int i = 0; i < currentNode->mNumMeshes; i++) {
                    aiMesh *mesh = scene->mMeshes[currentNode->mMeshes[i]];
                    meshes.push_back(processMesh(mesh, scene));
                }

                for(unsigned int i = 0; i < currentNode->mNumChildren; i++) {
                    nodes.push_back(currentNode->mChildren[i]);
                }
            }

        }

Mesh processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // process vertex positions, normals and texture coordinates
    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        glm::vec3 vector;
        
        // positions
        if(mesh->HasPositions()) {
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
        } else {
            vertex.Position = glm::vec3(0.0f);
        }

        // normals
        if (mesh->HasNormals()) {
            vertex.Normal = glm::vec3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );
        } else {
            vertex.Normal = glm::vec3(0.0f);
        }

        // texture coordinates
        if(mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        } else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }
        
        vertices.push_back(vertex);
    }
    
    // process indices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }


    // process material
    if(mesh->mMaterialIndex >= 0) {
        
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());


    }
    return Mesh(vertices, indices, textures);
}

    std::vector<Texture> loadMaterialTextures(aiMaterial *material, aiTextureType type, std::string typeName) {
        std::vector<Texture> textures;

        for(unsigned int i = 0; i < material->GetTextureCount(type); i++) {
            aiString str;
            material->GetTexture(type, i, &str);
            

            // check if texture was loaded before
            bool skip = false;

            for(unsigned int j = 0; j < loadedTextures.size(); j++) {
                if(std::strcmp(loadedTextures[j].path.c_str(), str.C_Str()) == 0) {
                    textures.push_back(loadedTextures[j]);
                    skip = true;
                    break;
                }
            }

            Texture texture;
            
            if(!skip) {
                std::string filename = std::string(str.C_Str());
                filename = directory + '/' + filename;
                
                texture.id = createTexture(filename.c_str());

                texture.type = typeName;
                texture.path = std::string(str.C_Str());
                
                textures.push_back(texture);
                loadedTextures.push_back(texture);
            }

            
        }

        return textures;
    }

};

#endif // MODEL_H
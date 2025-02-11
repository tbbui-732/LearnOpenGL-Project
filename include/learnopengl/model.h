//#include "assimp/Importer.hpp"
//#include "assimp/scene.h"
//#include "assimp/postprocess.h"

// NOTE: DELETE ONCE FINISHED. FOR LSP PURPOSES
#include <string>
#include <iostream>
#include "../stb_image/stb_image.h"

#include "../assimp/Importer.hpp"
#include "../assimp/scene.h"
#include "../assimp/postprocess.h"

class Model {
private:
    std::string directory;
    std::vector<Mesh> meshes;

    unsigned int TextureFromFile(char *str, std::string directory) {
        // TODO !
        // ...
    }

    std::vector<Texture> loadMaterialTextures(aiMaterial* material,
            aiTextureType type, std::string typeName)
    {
        std::vector<Texture> textures;
        for (int i = 0; i < material->GetTextureCount(type); ++i) {
            aiString str;
            material->GetTexture(type, i, &str);

            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str;

            textures.push_back(texture);
        }

        return textures;
    }

    void loadModel (std::string path) {
        Assimp::Importer importer;

        // Convert non-triangles to triangle primitives, and flips image around y-axis
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        // Check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ASSIMP::ERROR::" << importer.GetErrorString() << std::endl;
            exit(1);
        }

        directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode *node, const aiScene *scene) {
        // process node's meshes
        for (int i = 0; i < node->mNumMeshes; ++i) {
            int meshIdx = node->mMeshes[i];
            aiMesh *mesh = scene->mMeshes[meshIdx];
            meshes.push_back(processMesh(mesh, scene));
        }

        // recursively process node's children's meshes too
        for (int i = 0; i < node->mNumChildren; ++i) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        // process vertex positions (position, normal, texcoords)
        for (int i = 0; i < mesh->mNumVertices; ++i) {
            Vertex vertex;

            glm::vec3 pos;
            glm::vec3 norm;
            glm::vec3 texcoord;

            pos.x = mesh->mVertices[i].x;
            pos.y = mesh->mVertices[i].y;
            pos.z = mesh->mVertices[i].z;

            norm.x = mesh->mNormals[i].x;
            norm.y = mesh->mNormals[i].y;
            norm.z = mesh->mNormals[i].z;

            glm::vec2 vec = glm::vec2(0.0f, 0.0f);
            if (mesh->mTextureCoords[0]) {
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
            }

            vertex.Position = pos;
            vertex.Normal = norm;
            vertex.TexCoords = vec;

            vertices.push_back(vertex);
        }

        // process indices
        for (int i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            for (int j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // process material
        if (mesh->mMaterialIndex >= 0) {
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

            // process diffuse maps
            std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
                    aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

            // process specular maps
            std::vector<Texture> specularMaps = loadMaterialTextures(material,
                    aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }

        return Mesh(vertices, indices, textures);
    }
public:
};

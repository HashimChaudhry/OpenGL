#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "stb_image.h"
#include "Mesh.h"
#include "Shader.h"

using namespace std;
using namespace glm;
using namespace Assimp;

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

class Model {
public:
	// Model data
	vector<Texture> textures_loaded;	// Stores all textures loaded so far, optimization to make sure we don't keep loading duplicate textures
	vector<Mesh> meshes;				// Vector to keep track of all meshes in the object
	string directory;					// Directory of file
	bool gammaCorrection;				// Boolean for gamma correction

	// Constructor, expects a filepath to a 3D model
	Model(const string& path, bool gamma = false) : gammaCorrection(gamma) {
		loadModel(path);
	}

	// Draws the model (thus all of its meshes)
	void Draw(Shader& shader) {
		for (unsigned int i = 0; i < meshes.size(); i++) {
			meshes[i].Draw(shader);
		}
	}

private:`
	// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector
	void loadModel(const string& path) {
		// Importer object to import file and load the scene
		Importer importer; 
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		
		// Check for errors
		if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}

		// Retrieve the directory path of the file path
		directory = path.substr(0, path.find_last_of('/'));

		// Proccess ASSSIMP's root node recursively
		processNode(scene->mRootNode, scene);
	}

	void processNode(aiNode* node, const aiScene* scene) {
		// Process each mesh located at the current node
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			// The node object only contains indices to index the actual objects in the scene.
			// The scene contains all the data, node is just to kee stuff organized (like relations
			// between parent and and child meshes, etc.)
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}

		// After we've process all of the meshes, recurse through each of the child nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
		// Data to fill for mesh object
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

		// Walk through each of the mesh's vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex; // Contains vertex data of one vertex
			vec3 vector; // Delcare a placeholder vector since assimp uses its own vector class that doesn't cast well to glm
			
			// Positions
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			// Normals
			if (mesh->HasNormals()) {
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}

			// Texture Coordinates
			if(mesh->mTextureCoords[0]) { // Check to see if we have texture coordinates
				vec2 vec; // Placeholder vector

				// A vertex can contain up to 8 differe texture coordinates. We thus make the assumption that we won't
				// use models where a vertex can have multiple texture coordinates so we always take the first set (at index 0)
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;

				// Tangent
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = vector;

				// Bitangent
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.Bitangent = vector;

			} else {
				vertex.TexCoords = vec2(0.0f, 0.0f);
			}

			vertices.push_back(vertex);
		}

		// Now, we walk through each of the mesh's faces (a face is a triangle that
		//  makes up the mesh) and retrieve the correspding vertex indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];

			// Retrieve all indices for the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
			}
		}

		// Process the materials
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// We assume a convention for sampler names in the shaders. Each diffuse texture should be named
		// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
		// Same applies to other texture as the following list summarizes:
		// diffuse: texture_diffuseN
		// specular: texture_specularN
		// normal: texture_normalN

		// Diffuse maps
		vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		
		// Specular Maps
		vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		// Normal Maps
		vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		
		// Height Maps
		vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		// return a mesh object created from the extracted mesh data
		return Mesh(vertices, indices, textures);
	}

	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
		vector<Texture> textures;
		bool skip = false;

		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str;
			mat->GetTexture(type, i, &str);

			// Check if texture was loaded before and if so, continue to next iteration; skip loading a new texture
			for (unsigned int j = 0; j < textures_loaded.size(); j++) {
				if (strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			
			if(!skip) { // If texture hasn't been loaded already
				Texture texture;
				
				// Set up texture attributes
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str.C_Str();

				// Push texture into vector
				textures.push_back(texture);
				textures_loaded.push_back(texture); // Push vector into loaded textures vector
			}
		}

		return textures;
	}
};

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma) {
	string filename = string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

	if(data) { // Check to see if image data was read
		GLenum format;
		if (nrComponents == 1) {
			format = GL_RED;
		} else if (nrComponents == 3) {
			format = GL_RGB;
		} else if (nrComponents == 4) {
			format = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	} else {
		cout << "Texture failed to load at path: " << path << endl;
		stbi_image_free(data);
	}

	return textureID;
}

#endif

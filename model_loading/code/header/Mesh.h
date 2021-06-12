#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // Holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>

#include "Shader.h"

using namespace std;
using namespace glm;

struct Vertex {
	vec3 Position;	// Position
	vec3 Normal;	// Normal
	vec2 TexCoords;	// Texture Coordinates
	vec3 Tangent;	// Tangent
	vec3 Bitangent;	// Bitangent
};

struct Texture {
	unsigned int id;
	string type;
	string path;
};

class Mesh {
public:
	// Mesh data
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	unsigned int VAO;

	// Constructor
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures) {
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		// Now that we have all the data required, set the vertex buffers and its attribute pointers
		setupMesh();
	}

	// Render the mesh
	void Draw(Shader& shader) {
		// Bind the appropriate textures 
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		unsigned int normalNr = 1;
		unsigned int heightNr = 1;

		for (unsigned int i = 0; i < textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i); // Activate proper texture unit before binding
			
			// Retrieve texture number
			string number;
			string name = textures[i].type;

			// Identify type of texture
			if (name == "texture_diffuse") {
				number = to_string(diffuseNr++);
			} else if (name == "texture_specular") {
				number = to_string(specularNr++);
			} else if (name == "texture_normal") {
				number = to_string(normalNr++);
			} else if (name == "texture_height") {
				number = to_string(heightNr++);
			}

			// Now, set the samper to the correct texture unit
			glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
			
			// And finally bind the texture
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}

		// Draw mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0); // Unbind VAO

		// Set everything up back to defaults
		glActiveTexture(GL_TEXTURE0);
	}

private:
	// Rendering data
	unsigned int VBO, EBO;

	// Initialize all buffer objects and arrays
	void setupMesh() {
		// Create buffers/arrays
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		// Bind VAO
		glBindVertexArray(VAO);

		// Load data into vetex buffers
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		// A great thing about structs is that their memory layout is sequential for all its items.
		// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a vec3/2 array which
		// again translates to 3/2 floats which translates to a byte array.
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		
		// Setup vertex position attribute
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		
		// Setup vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		
		// Set up vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
		
		// Set up vertex tangent
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

		// Set up vertex tangent
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3,GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

		// Reset to defaults
		glBindVertexArray(0);
		
	}
};

#endif

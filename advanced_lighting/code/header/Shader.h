#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // Include glad to get the required OpenGL headers
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;
using namespace glm;

class Shader {

public:
	// The program ID
	unsigned int ID;

	// Constructor reads and builds the shader
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);

	// Use or activate the shader
	void use();

	// Utility uniform functions
	void setBool(const string& name, bool value) const;
	void setInt(const string& name, int value) const;
	void setFloat(const string& name, float value) const;

	// GLM utility uniform functions for vectors
	void setVec2(const string& name, const vec2& value) const;
	void setVec2(const string& name, float x, float y) const;
	void setVec3(const string& name, const vec3& value) const;
	void setVec3(const string& name, float x, float y, float z) const;
	void setVec4(const string& name, const vec4& value) const;
	void setVec4(const string& name, float x, float y, float z, float w) const;

	// GLM utility uniform functions for matrices
	void setMat2(const string& name, const mat2& mat) const;
	void setMat3(const string& name, const mat3& mat) const;
	void setMat4(const string& name, const mat4& mat) const;

private:
	// Utility function for checking the shader compiling and linking errors
	void checkCompileErrors(GLuint shader, string type);
};

#endif // !SHADER_H
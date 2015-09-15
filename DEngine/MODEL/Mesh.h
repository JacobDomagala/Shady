#ifndef MESH_H
#define MESH_H


#include <sstream>
#include <vector>

#include <gtc/matrix_transform.hpp>
#include"../SHADERS/Shader.h"
#include"Texture.h"


using std::vector;
using std::string;
using glm::vec2;
using glm::vec3;

enum textureType {
	DIFFUSE,
	SPECULAR,
	NORMAL
};



struct Vertex {
	vec3 position;
	vec3 normal;
	vec2 texCoords;
	vec3 tangent;
};

class Mesh {
public:
	
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;
	GLuint programID;

	
	Mesh(vector<Vertex>* vertices, vector<GLuint>* indices, vector<Texture>* textures);
	void AddTexture(char* filePath, textureType textureType);

	void Draw(GLuint programID);
	void Delete();

private:
	
	GLuint VAO, VBO, EBO;

	void SetupMesh();

};


#endif
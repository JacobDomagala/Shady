#ifndef MESH_H
#define MESH_H
// Std. Includes

#include <sstream>
#include <vector>

// GL Includes

#include <gtc/matrix_transform.hpp>
#include"../SHADERS/Shader.h"
#include"Texture.h"


using std::vector;
using std::string;
using glm::vec2;
using glm::vec3;

struct Vertex {
	vec3 position;
	vec3 normal;
	vec2 texCoords;
	vec3 tangent;
	vec3 bTangent;
};

class Mesh {
public:
	/*  Mesh Data  */
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;
	GLuint programID;

	/*  Functions  */
	// Constructor
	Mesh(vector<Vertex>* vertices, vector<GLuint>* indices, vector<Texture>* textures);

	// Render the mesh
	void Draw(GLuint programID);
	void Delete();

private:
	/*  Render data  */
	GLuint VAO, VBO, EBO;

	/*  Functions    */
	// Initializes all the buffer objects/arrays
	void SetupMesh();

};


#endif
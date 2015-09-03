#ifndef MESH_H
#define MESH_H
// Std. Includes

#include <sstream>
#include <vector>
using namespace std;
// GL Includes

#include <gtc/matrix_transform.hpp>
#include"../SHADERS/Shader.h"
#include"Texture.h"


struct Vertex {

	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
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
	void Draw(Shader shader);


private:
	/*  Render data  */
	GLuint VAO, VBO, EBO;

	/*  Functions    */
	// Initializes all the buffer objects/arrays
	void setupMesh();

};


#endif
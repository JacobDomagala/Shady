#ifndef MESH_H
#define MESH_H

#include<gtc/type_ptr.hpp>

#include"..\DISPLAY\Movement.h"
#include"..\DISPLAY\Display.h"
#include"..\3RD_PARTY\ObjLoader.h"
#include"Texture.h"

using glm::vec2;
using glm::vec3;
using glm::mat4;

class Mesh{
private:
	
	float* fPyramid;
	int iAttrBitField;
	Texture* texture;

	GLuint mvpLocation;
	GLuint modelMatrixLocation;
	GLuint lightPositionLocation;
	GLuint cameraPositionLocation;
	GLuint shinePowerLocation;
	GLuint timeLocation;
	GLuint materialLocation;

	mat4 MVP;
	mat4 modelMatrix;
	mat4 translateMatrix;
	mat4 rotateMatrix;
	mat4 scaleMatrix;


	vec3* positions;
	vec3* normals; 
	vec2* uvs;
	
	GLushort* indicies;
	float shinePower;
	


	
	unsigned int vertexBufferSize;


	GLuint numVertices;
	GLuint numIndices;

	enum attribs
	{
		POSITION,
		UV,
		NORMAL,
		INDEX,
		NUM_BUFFERS
	};

	GLuint vertexBuffers[NUM_BUFFERS];
	
	GLuint vertexArrayID;
	
public:
	
	
	void InitCube();
	void RenderCube(Shader* program,
		Movement* camera,
		Display* window,
		float shinePower,
		vec3 scale,
		vec3 translate,
		float angle,
		vec3 rotation);

	GLsizeiptr VertexBufferSize(){
		return numVertices * sizeof(vec3);
	}


	void Clean() {
		delete[] positions;
		delete[] uvs;
		delete[] normals;
	
		numVertices = 0;
	}
	
	void LoadShape(const char * path);

	void Render(Shader* program, 
				Movement* camera,
				Display* window,
				float shinePower,
				vec3 scale,
				vec3 translate, 
				float angle,
				vec3 rotation);
	
	void InitMesh(const IndexedModel& model);
};

#endif
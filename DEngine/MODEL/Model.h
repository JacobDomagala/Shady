#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>


#include <glew.h> 
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <SOIL.h>
#include <Importer.hpp>
#include <scene.h>


#include "Mesh.h"
#include"Texture.h"
#include"../DISPLAY/Camera.h"


using std::string;
using std::vector;
using glm::mat4;
using glm::vec3;

class Model{
public:
	
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	

	vec3 translateValue;
	vec3 rotateValue;
	vec3 scaleValue;
	float rotateAngle;



	Model(GLchar* path);
	void ScaleModel(vec3 scale);
	void TranslateModel(vec3 translate);
	void RotateModel(vec3 rotate, float angle);



	void Draw(Display* window, Camera camera, Shader normalShader);
	void DrawToDepthBuffer(Display* window, vec3 lightPos, Shader shadowShader);

	GLuint modelMatrixUniformLocation;

	GLuint programID;
	vector<Mesh> meshes;
	string directory;
	vector<Texture> textures_loaded;	// Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.

										/*  Functions   */
										// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void LoadModel(string path);
	// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void ProcessNode(aiNode* node, const aiScene* scene);

	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);


	// Checks all material textures of a given type and loads the textures if they're not loaded yet.
	// The required info is returned as a Texture struct.
	vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, char* typeName);
};




#endif
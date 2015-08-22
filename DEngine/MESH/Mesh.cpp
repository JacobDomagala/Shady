#include "Mesh.h"

void  Mesh::LoadShape(const char * path) {
	InitMesh(OBJModel(path).ToIndexedModel());
}
void Mesh::InitCube() {

	GLfloat vertices[] = {
		// Positions          
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
	};
	GLfloat normalz[]{
		0.0f,  0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	GLfloat UVs[]{
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,

		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,

		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f
	};
	
	size_t size = sizeof(vertices) / sizeof(GLfloat);
	positions = new vec3[size/3];
	memcpy(positions, vertices, sizeof(vertices));
	
	size = sizeof(normalz) / sizeof(GLfloat);
	normals = new vec3[size / 3];
	memcpy(normals, normalz, sizeof(normalz));

	size = sizeof(UVs) / sizeof(GLfloat);
	uvs = new vec2[size / 2];
	memcpy(uvs, UVs, sizeof(UVs));

	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);
	glGenBuffers(NUM_BUFFERS, vertexBuffers);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[POSITION]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[UV]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[NORMAL]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
}
void Mesh::RenderCube(Shader* program, Movement* camera, Display* window,
					  float shinePower, vec3 scale, vec3 translate,
	                  float angle, vec3 rotation) {


}
	void Mesh::InitMesh(const IndexedModel& model)
	{
		numIndices = model.indices.size();
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);
		glGenBuffers(NUM_BUFFERS, vertexBuffers);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[POSITION]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(model.positions[0]) * model.positions.size(), &model.positions[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[UV]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(model.texCoords[0]) * model.texCoords.size(), &model.texCoords[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[NORMAL]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(model.normals[0]) * model.normals.size(), &model.normals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffers[INDEX]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(model.indices[0]) * model.indices.size(), &model.indices[0], GL_STATIC_DRAW);

		glBindVertexArray(0);
	}


void Mesh::Render(Shader* program, Movement* camera, Display* window, float shinePower,
		          vec3 scaleValue, vec3 translate, float angle, vec3 rotation) {

		vec3 cameraPosition = camera->GetPosition();
		vec3 lightPosition = camera->GetLightPosition();




		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glUseProgram(program->GetProgramID());
		glBindVertexArray(vertexArrayID);
		mvpLocation = glGetUniformLocation(program->GetProgramID(), "MVP");
		modelMatrixLocation = glGetUniformLocation(program->GetProgramID(), "modelMatrix");
		lightPositionLocation = glGetUniformLocation(program->GetProgramID(), "lightPosition");
		cameraPositionLocation = glGetUniformLocation(program->GetProgramID(), "cameraPosition");
		shinePowerLocation = glGetUniformLocation(program->GetProgramID(), "meshMaterial.shinePower");
		timeLocation = glGetUniformLocation(program->GetProgramID(), "time");


		mat4 scaleMatrix = glm::scale(scaleValue);
		mat4 rotateMatrix = glm::rotate(angle, rotation);
		mat4 translateMatrix = glm::translate(translate);
		mat4 modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;
		mat4 MVP = window->GetProjection() * camera->GetWorldToViewMatrix()* modelMatrix;

		glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniform3fv(lightPositionLocation, 1, &lightPosition[0]);
		glUniform3fv(cameraPositionLocation, 1, &cameraPosition[0]);
		glUniform1f(shinePowerLocation, shinePower);
		glUniform1f(timeLocation, camera->GetTime());

		glPointSize(3.0);
		if(numIndices)glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
		else glDrawArrays(GL_TRIANGLES, 0, 36);


	}


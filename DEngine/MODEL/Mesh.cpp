#include"Mesh.h"

Mesh::Mesh(vector<Vertex>* vertices, vector<GLuint>* indices, vector<Texture>* textures)
{
	this->vertices = *vertices;
	this->indices = *indices;
	this->textures = *textures;

	// Now that we have all the required data, set the vertex buffers and its attribute pointers.
	SetupMesh();
}

void Mesh::AddTexture(char* filePath, textureType textureType)
{
	Texture tmp;
	switch (textureType) 
	{
		case 0:
		{
			tmp.LoadTexture(filePath, "diffuse_map");
			break;
		}
		case 1:
		{
			tmp.LoadTexture(filePath, "specular_map");
			break;
		}
		case 2:
		{
			tmp.LoadTexture(filePath, "normal_map");
			break;
		}
	}
	textures.push_back(tmp);

}

void Mesh::Draw(GLuint programID)
{	
	if (programID != NULL)
	{
		for (GLuint i = 0; i < textures.size(); i++)
		{
			textures[i].Use(programID, i);
		}
	}
	// Draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// Always good practice to set everything back to defaults once configured.
	if (programID != NULL)
	{
		for (GLuint i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}
void Mesh::SetupMesh()
{
	// Create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// A great thing about structs is that their memory layout is sequential for all its items.
	// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
	// again translates to 3/2 floats which translates to a byte array.
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	// Set the vertex attribute pointers
	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	// Vertex Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
	// Vertex Texture Coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texCoords));
	// Vertex Tangents
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, tangent));

	glBindVertexArray(0);
}
void Mesh::Delete()
{
	vertices.clear();
	textures.clear();
	indices.clear();
}
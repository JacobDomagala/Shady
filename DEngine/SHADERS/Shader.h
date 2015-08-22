#ifndef SHADER_H
#define SHADER_H

#include<string>
#include<iostream>
#include<fstream>
#include<glew.h>
#include<glm.hpp>

class Shader
{
public:
	Shader() {}
	void LoadShaders(char* vertexFile, char* fragmentFile);
	~Shader();

	GLuint GetProgramID();
	void UseProgram();
private:
	
	GLuint m_Program;

	bool CheckStatus(GLuint, PFNGLGETSHADERIVPROC, PFNGLGETSHADERINFOLOGPROC, GLenum);
	bool CheckShaderStatus(GLuint);
	bool CheckProgramStatus(GLuint);
	
	char* ReadFile(char* fileName);
};

#endif
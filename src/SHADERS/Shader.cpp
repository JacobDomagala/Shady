#include "Shader.h"

void
Shader::AddTess()
{
   const char* shaderSource;
   GLuint tessControlShader = glCreateShader(GL_TESS_CONTROL_SHADER);
   GLuint tessEvalShader = glCreateShader(GL_TESS_EVALUATION_SHADER);

   shaderSource = ReadFile("./box.tcs");
   glShaderSource(tessControlShader, 1, &shaderSource, 0);

   shaderSource = ReadFile("./box.tes");
   glShaderSource(tessEvalShader, 1, &shaderSource, 0);

   glCompileShader(tessControlShader);
   glCompileShader(tessEvalShader);

   glAttachShader(programID, tessControlShader);
   glAttachShader(programID, tessEvalShader);
   glLinkProgram(programID);
}
void
Shader::LoadShaders(char* vertexFile, char* fragmentFile)
{
   GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
   GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);


   const char* shaderSource;
   shaderSource = ReadFile(vertexFile);

   glShaderSource(vertexShader, 1, &shaderSource, 0);

   shaderSource = ReadFile(fragmentFile);
   glShaderSource(fragmentShader, 1, &shaderSource, 0);

   glCompileShader(vertexShader);
   glCompileShader(fragmentShader);

   if (!CheckShaderStatus(vertexShader) || !CheckShaderStatus(fragmentShader))
   {
      system("pause");
   }

   programID = glCreateProgram();

   glAttachShader(programID, vertexShader);
   glAttachShader(programID, fragmentShader);

   glLinkProgram(programID);

   if (!CheckProgramStatus(programID))
   {
      return;
   }
}

char*
Shader::ReadFile(char* fileName)
{
   FILE* file;
   char* buffer;
   unsigned int size;

   file = fopen(fileName, "rb");
   if (!file)
   {
      std::cerr << "Shader file" << fileName << "couldn't be opened\n";
      system("pause");
   }
   fseek(file, 0, SEEK_END);
   size = ftell(file);
   buffer = (char*)malloc(size);
   rewind(file);

   unsigned int offset = 0;
   while (!feof(file))
   {
      fgets(buffer + offset, size, file);
      offset = ftell(file);
   }
   fclose(file);
   return buffer;
}

bool
Shader::CheckShaderStatus(GLuint shader)
{
   return CheckStatus(shader, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}
bool
Shader::CheckProgramStatus(GLuint program)
{
   return CheckStatus(program, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

bool
Shader::CheckStatus(GLuint object, PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
                    PFNGLGETSHADERINFOLOGPROC getInfoLogFunc, GLenum statusType)
{
   GLint Status;
   objectPropertyGetterFunc(object, statusType, &Status);
   if (Status != GL_TRUE)
   {
      GLint infoLogLength;
      objectPropertyGetterFunc(object, GL_INFO_LOG_LENGTH, &infoLogLength);
      GLchar* buffer = new GLchar[infoLogLength];

      GLsizei bufferSize;
      getInfoLogFunc(object, infoLogLength, &bufferSize, buffer);
      std::cout << buffer << std::endl;

      delete[] buffer;
      return false;
   }
   return true;
}

void
Shader::UseProgram()
{
   glUseProgram(0);
   glUseProgram(programID);
}
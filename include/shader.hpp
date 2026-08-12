#pragma once
#include <glad/glad.h>
#include <string>

class Shader {
public:
  Shader();
  ~Shader();

  Shader(const Shader& other) = delete;
  Shader& operator=(const Shader& other) = delete;

  void use();
  void unuse();
  void setMat4(const std::string& name, const float* matrixData);

private:
  // path is relative to current working directory of the process at runtime
  std::string getShaderSource(const std::string& path);
  GLuint createShaderProgram(const std::string& vertexShaderSource,
                             const std::string& fragmentShaderSource);
  GLuint compile(GLuint type, const std::string& shaderSource);
  void verifyCompilation(GLuint shaderObj);

  GLuint program_{0};
};

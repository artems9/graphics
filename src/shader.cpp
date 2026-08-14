#include "shader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

Shader::Shader() {
  const std::string vertexShaderSource = getShaderSource("./shaders/vert.glsl");
  const std::string fragmentShaderSource =
      getShaderSource("./shaders/frag.glsl");

  program_ = createShaderProgram(vertexShaderSource, fragmentShaderSource);
}

Shader::~Shader() { glDeleteProgram(program_); }

void Shader::use() { glUseProgram(program_); }

void Shader::unuse() { glUseProgram(0); }

// uniform var name must match the one declared across shaders
// this function basically transfers data from CPU to GPU
void Shader::setMat4(const std::string& name, const float* matrixData) {
  GLint location = glGetUniformLocation(program_, name.c_str());
  if (location < 0) {
    throw std::runtime_error("Uniform not found: " + name);
  }
  glUniformMatrix4fv(location, 1, GL_FALSE, matrixData);
}

// path is relative to current working directory of the process at runtime
std::string Shader::getShaderSource(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open shader file: " + path);
  }

  std::stringstream buffer;
  // get whole file into stream
  buffer << file.rdbuf();
  return buffer.str();
}

GLuint Shader::createShaderProgram(const std::string& vertexShaderSource,
                                   const std::string& fragmentShaderSource) {
  GLuint programObj = glCreateProgram();

  GLuint vertexShader = compile(GL_VERTEX_SHADER, vertexShaderSource);

  GLuint fragmentShader = compile(GL_FRAGMENT_SHADER, fragmentShaderSource);

  glAttachShader(programObj, vertexShader);
  glAttachShader(programObj, fragmentShader);
  glLinkProgram(programObj);

  GLint linked{0};
  glGetProgramiv(programObj, GL_LINK_STATUS, &linked);
  if (linked == GL_FALSE) {
    GLint msgLength{0};
    glGetProgramiv(programObj, GL_INFO_LOG_LENGTH, &msgLength);
    std::string errorMsg(msgLength, '\0');
    glGetProgramInfoLog(programObj, msgLength, nullptr, errorMsg.data());
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(programObj);
    throw std::runtime_error("Shader program link failed:\n" + errorMsg);
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return programObj;
}

GLuint Shader::compile(GLuint type, const std::string& shaderSource) {
  GLuint shaderObj;

  // Create shader object specifically for the type passed in
  if (type == GL_VERTEX_SHADER) {
    shaderObj = glCreateShader(GL_VERTEX_SHADER);
  } else if (type == GL_FRAGMENT_SHADER) {
    shaderObj = glCreateShader(GL_FRAGMENT_SHADER);
  }

  const char* src{shaderSource.c_str()};
  glShaderSource(shaderObj, 1, &src, nullptr);
  glCompileShader(shaderObj);
  verifyCompilation(shaderObj);

  return shaderObj;
}

void Shader::verifyCompilation(GLuint shaderObj) {
  GLint compilationResult;
  glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &compilationResult);

  if (compilationResult == GL_FALSE) {
    // find require buffer size for the info log
    GLint msgLength;
    glGetShaderiv(shaderObj, GL_INFO_LOG_LENGTH, &msgLength);
    // allocate buffer for outputting error message
    std::string errorMsg(msgLength, '\0');
    // Retrieve the shader compiler's error log.
    glGetShaderInfoLog(shaderObj, msgLength, nullptr, errorMsg.data());
    // delete broken shader
    glDeleteShader(shaderObj);
    throw std::runtime_error("Shader compilation failed:\n" + errorMsg);
  }
}

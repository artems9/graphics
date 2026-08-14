#pragma once
#include <glad/glad.h>

class Mesh {
public:
  Mesh();
  ~Mesh();

  Mesh(const Mesh& other) = delete;
  Mesh& operator=(const Mesh& other) = delete;

  void draw();

private:
  // triangle
  // Vertex Array Object (VAO) - How to interpret the data.
  // Stores the vertex attribute layout and the VBO/IBO bindings needed to
  // interpret vertex data.
  GLuint vao_{0};
  // Vertex Buffer Object (VBO) - What the data is.
  // Stores vertex data (positions, colors, UVs, etc.) in GPU memory.
  GLuint vbo_{0};
  // Index Buffer Object (IBO) - Which vertices to draw.
  // Stores indices that define how vertices are reused to form primitives.
  GLuint ibo_{0};
};

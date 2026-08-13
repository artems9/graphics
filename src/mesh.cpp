#include "mesh.hpp"
#include <vector>

Mesh::Mesh() {
    // OPENGL uses counter clockwise winding order by default
    // rendering QUAD
    const std::vector<GLfloat> vertexData{
        // clang-format off
        // Vertex 0 (bottom left)
        -0.5f, -0.5f, 0.0f, // position x, y, z
        1.0f, 0.0f, 0.0f,   // color    r, b, b
        // Vertex 1 (bottom right)
        0.5f, -0.5f, 0.0f, // position
        0.0f, 1.0f, 0.0f, // color
        // Vertex 2 (top right)
        0.5f, 0.5f, 0.0f, // position
        0.0f, 0.0f, 1.0f, // color
        // Vertex 3 (top left)
        -0.5f, 0.5f, 0.0f, // position
        1.0f, 0.0f, 1.0f, // color
        // clang-format on
    };

    // generate x amount of VAO (instructions for reading the VBO)
    glGenVertexArrays(1, &vao_);

    // select VAO to use
    glBindVertexArray(vao_);

    //===================/ SETUP VBO /===================
    // generate x amount of VBO (actual vertex data)
    glGenBuffers(1, &vbo_);

    // select VBO to use
    // GL_ARRAY_BUFFER can be thought of as:
    // A pointer (or handle) currently selected VBO.
    // so subsequent functions operating on GL_ARRAY_BUFFER refer to bound vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    // populate target buffer used by GPU, with data from VBO (vertexPositions)
    glBufferData(GL_ARRAY_BUFFER,                     // target buffer
                 vertexData.size() * sizeof(GLfloat), // size in bytes
                 vertexData.data(),                   // pointer to data
                 GL_STATIC_DRAW);                     // usage type

    const std::vector<GLuint> iboData{0, 1, 3, 1, 2, 3};

    // setup IBO (Index Buffer Object) or EBO (Element Buffer Object)
    glGenBuffers(1, &ibo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    // populate IBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 iboData.size() * sizeof(GLuint),
                 iboData.data(),
                 GL_STATIC_DRAW);

    // ================== SETUP VAO POSITION =====================
    // populate the currently bound VAO with instructions
    // by telling opengl how to interpret currently bound VBO
    glVertexAttribPointer(
        0,        // attribute index, corresponds to glEnableVertexAttribArray
        3,        // num. components per attribute (x, y, z)
        GL_FLOAT, // type
        GL_FALSE, // data value normalization
        sizeof(GLfloat) * 6, // stride (byte offset between attribute)
        nullptr // offset into the bound VBO where this attribute begins
    );
    // Enable attributes for input to the vertex shader.
    glEnableVertexAttribArray(0);

    // ================== SETUP VAO COLOR =====================
    glVertexAttribPointer(
        1,        // attribute index, corresponds to glEnableVertexAttribArray
        3,        // num. components per attribute (r, g, b)
        GL_FLOAT, // type
        GL_FALSE, // data value normalization
        sizeof(GLfloat) * 6, // stride (byte offset between attribute)
        reinterpret_cast<const void*>(
            sizeof(GLfloat) *
            3) // offset into the bound VBO where this attribute begins
    );
    // Enable attribute for input to the vertex shader.
    glEnableVertexAttribArray(1);

    // Unbind currently bound VAO
    glBindVertexArray(0);
    // Disable any attributes previously enabled in VAO
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

Mesh::~Mesh() {
  glDeleteBuffers(1, &ibo_);
  glDeleteBuffers(1, &vbo_);
  glDeleteVertexArrays(1, &vao_);
}

void Mesh::draw() {
  // enable attributes
  glBindVertexArray(vao_);
  // render indexed data
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

#pragma once

#include "glm/glm.hpp"

class Camera {
  public:
    // Default Constructor
    Camera();
    // Produces and returns 4x4 view matrix
    glm::mat4 getViewMatrix() const;

    void moveForward(float speed);
    void moveBackward(float speed);
    void moveLeft(float speed);
    void moveRight(float speed);
  
  private:
    glm::vec3 eye_;
    glm::vec3 viewDir_;
    glm::vec3 upVec_;
};


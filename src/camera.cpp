#include "camera.hpp"
#include "glm/gtc/matrix_transform.hpp"

// Constructor
Camera::Camera() {
  // Assume we are placed at the origin
  eye_      = glm::vec3(0.0f, 0.0f, 0.0f);

  // Asume we are looking out into the world
  // Note: this is along -z, else we'd be looking behind us
  viewDir_  = glm::vec3(0.0f, 0.0f, -1.0f);

  // Assume we start on a perfect plane
  upVec_    = glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::mat4 Camera::getViewMatrix() const {
  return glm::lookAt(eye_, viewDir_, upVec_);
}

void Camera::moveForward(float speed) {
  eye_.z -= speed;
}

void Camera::moveBackward(float speed) {
    eye_.z += speed;
  }

void Camera::moveLeft(float speed) {
    eye_.x += speed;
  }

void Camera::moveRight(float speed) {
    eye_.x -= speed;
  }

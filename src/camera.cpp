#define GLM_ENABLE_EXPERIMENTAL // opt into gtx/rotate_vector.hpp, etc.
#include "camera.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include <iomanip>
#include <iostream>

// Constructor
Camera::Camera() {
  // Assume we are placed at the origin
  camPos_ = glm::vec3(0.0f, 0.0f, 0.0f);

  // Asume we are looking out into the world
  // Note: this is along -z, else we'd be looking behind us
  viewDir_ = glm::vec3(0.0f, 0.0f, -1.0f);

  // Assume we start on a perfect plane (aligned with Y axis pointing upwards)
  upVec_ = glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::mat4 Camera::getViewMatrix() const {
  return glm::lookAt(camPos_, camPos_ + viewDir_, upVec_);
}

void Camera::moveMouse(float x, float y) {
  constexpr float kMouseSens{0.05f};
  // rotate viewDir_ by x radians
  viewDir_ = glm::rotate(viewDir_, glm::radians(-x * kMouseSens), upVec_);
  viewDir_ = glm::normalize(viewDir_); // snap length back to 1
  // std::cout << std::fixed << std::setprecision(10) << glm::length(viewDir_) << '\n';

  /*
   * - Every time the mouse moves, you call glm::rotate(viewDir_, angle,
   * upVec_). In perfect math, rotating a vector never changes its length — only
   * its direction. But glm::rotate uses sin/cos internally, and computers can't
   * store most sin/cos results with perfect precision (floats only have so many
   * digits) — so each rotation is very slightly "off" from a mathematically
   * perfect one. That tiny imperfection can nudge the length up or down by a
   * microscopic amount, each time you rotate.
   *
   * - viewDir_ is supposed to represent only a direction — "which way am I
   * looking" — with the length itself carrying no meaning. glm::normalize
   * takes whatever direction the vector currently points and rescales it to
   * length 1, without changing which way it's pointing.
   */
}

void Camera::moveForward(float speed) { camPos_ += viewDir_ * speed; }

void Camera::moveBackward(float speed) { camPos_ -= viewDir_ * speed; }

void Camera::moveLeft(float speed) {
  glm::vec3 rightVec = glm::cross(viewDir_, upVec_);
  camPos_ -= rightVec * speed;
}

void Camera::moveRight(float speed) {
  glm::vec3 rightVec = glm::cross(viewDir_, upVec_);
  camPos_ += rightVec * speed;
}

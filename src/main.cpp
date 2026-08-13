// Third Party Libraries
#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// Standard Template Library
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <print>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
// My Libraries
#include "camera.hpp"
#include "window.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "clock.hpp"

struct SDLApplication {
  bool running_                             {true};

  // only used in updateFpsCounter() to display window title as fps count
  // TODO: make its own class 
  double fpsTimer_                          {0.0};
  int frameCount_                           {0};

  // will leave here for now but in the future will abstract into class?
  float u_offsetY_{0.0f};
  float u_offsetX_{0.0f};
  float u_offsetZ_{-3.0f}; // start at -3 for immediate visibility
  float u_rotate_{0.0f};
  float u_scale_{0.5f};

  Window  window_; 
  Shader  shader_;
  Camera  camera_;
  Mesh    mesh_;
  Clock   clock_;

  // infinite loop
  void run() {
    // start clock here to:
    // exclude SDL and OPENGL initialization and setup times
    // handle (edge case) first call in loop
    clock_.start();
    while (running_) {
      advanceFrame();
    }
  }

  // easy to add breakpoint for debugging
  void advanceFrame() {
    double frameTime = clock_.getFrameTime();

    // 1. get whatever user input to act on
    getInput();
    // 2. update using fixed-step (0 or more, depends on accumulatedTime_)
    updateWorld();
    // 3.
    renderFrame(/*using alpha interpolation?*/);
    // display frames in window title
    updateFpsCounter(frameTime);
  }

  void renderFrame() {
    setup();
    draw();
    window_.swapBuffers();
  }

  // set up OPENGL state
  void setup() {
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Initialize clear color (background of screen)
    glClearColor(1.f, 1.f, 0.f, 1.f);

    // Clear color and depth buffers
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    shader_.use();

    // ------------------- MODEL -----------------------

    // ORDER OF OPERATIONS MATTERS !!!!

    // Model transformation by translating our object into world space
    glm::mat4 model = glm::translate(
        glm::mat4(1.0f), glm::vec3(u_offsetX_, u_offsetY_, u_offsetZ_));

    // Update model matrix by applying rotation
    u_rotate_ -= 1.0f; // automatically rotate constantly
    model = glm::rotate(
        model, glm::radians(u_rotate_), glm::vec3(0.0f, 1.0f, 0.0f));

    // Update model matrix by applying rotation
    model = glm::scale(model, glm::vec3(u_scale_, u_scale_, u_scale_));

    shader_.setMat4("u_modelMatrix", &model[0][0]);

    // ------------------- CAMERA  -----------------------
    glm::mat4 view = camera_.getViewMatrix();
    shader_.setMat4("u_viewMatrix", &view[0][0]);
    // ------------------- PROJECTION -----------------------

    // Projection matrix (in perspective)
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f),
                                             800.0f / 400.0f,
                                             0.1f,   // how close we can see
                                             10.0f); // how far we can see

    shader_.setMat4("u_perspectiveProjection", &perspective[0][0]);
  }

  // OPENGL draw calls
  void draw() {
    mesh_.draw();
    // stop using current pipeline, unnecessary if we only have 1 pipleine
    shader_.unuse();
  }


  void updateFpsCounter(const double frameTime) {
    fpsTimer_ += frameTime;
    frameCount_++;
    if (fpsTimer_ >= 1.0) { // refresh once a second
      // frameCount_ / fpsTimer_ rather than averaging 1/frameTime each frame —
      // this gives you the true average FPS over that second (total frames
      // divided by total time), rather than an average-of-instantaneous-rates,
      // which behaves oddly when frame times vary (a few very fast or very slow
      // frames skew a naive average much more than they should).
      double fps = frameCount_ / fpsTimer_;
      window_.setTitle(("MyWindow - FPS: " + std::to_string(static_cast<int>(fps))).c_str());
      fpsTimer_ = 0.0;
      frameCount_ = 0;
    }
  }

  void updateWorld() {
    while (clock_.shouldStep()) {
      // explicit param for readability and future reusability
      // updateLogic/Physics/Anything(kFixedDt);
      clock_.consumeStep();
    }
  }


  void getInput() {
    SDL_Event event;
    // SDL_PollEvent() automatically calls SDL_PumpEvents() before checking
    // event queue, handles everything that happened since last frame
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running_ = false;

      } else if (event.type == SDL_EVENT_MOUSE_MOTION 
              && window_.isMouseCaptured()) {
        // only steer the camera while the mouse is captured
        camera_.moveMouse(event.motion.xrel, event.motion.yrel);

      } else if (event.type == SDL_EVENT_KEY_DOWN &&
                 event.key.scancode == SDL_SCANCODE_ESCAPE) {
        // one-shot: fires once per press, not every frame it's held
        // release the mouse so it can leave the window
        window_.setMouseCaptured(false);

      } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                 !window_.isMouseCaptured()) {
        // clicking back into the window re-captures the mouse
        window_.setMouseCaptured(true);
      } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
        window_.resize(event.window.data1, event.window.data2);
        }
    }
    // polls current state, every frame
    const bool* keys = SDL_GetKeyboardState(nullptr);

    // ============================= ARROW KEYS MOVE OBJECT ================
    // X axis (left)
    if (keys[SDL_SCANCODE_LEFT] == true) {
      u_offsetX_ -= 0.01f;
    }
    // X axis (right)
    if (keys[SDL_SCANCODE_RIGHT] == true) {
      u_offsetX_ += 0.01f;
    }

    // Y axis (up)
    if (keys[SDL_SCANCODE_UP] == true) {
      u_offsetY_ += 0.01f;
    }
    // Y axis (down)
    if (keys[SDL_SCANCODE_DOWN] == true) {
      u_offsetY_ -= 0.01f;
    }

    // ============================= WASD KEYS MOVE CAMERA ================

    float speed = 0.1f; // NOTE: temporary. make better abstraction later.

    // Z axis (forward)
    if (keys[SDL_SCANCODE_W] == true) {
      camera_.moveForward(speed);
    }
    // Z axis (backward)
    if (keys[SDL_SCANCODE_S] == true) {
      camera_.moveBackward(speed);
    }

    // X axis (left)
    if (keys[SDL_SCANCODE_A] == true) {
      camera_.moveLeft(speed);
    }
    // X axis (right)
    if (keys[SDL_SCANCODE_D] == true) {
      camera_.moveRight(speed);
    }
  }
};


// TODO: SDLLibrary RAII wrapper
int main(int argc, char* argv[]) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Init failed: %s", SDL_GetError());
    return EXIT_FAILURE; // SDL_Init never succeeded, nothing to quit
  }

  try {
    SDLApplication app;
    app.run();
  } catch (const std::exception& e) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error: %s", e.what());
    SDL_Quit();
    return EXIT_FAILURE;
  }

  SDL_Quit();
  return EXIT_SUCCESS;
}


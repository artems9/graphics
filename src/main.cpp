// Third Party Libraries
#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// Standard Template Library
#include <cstdlib>
#include <stdexcept>
#include <string>
// My Libraries
#include "camera.hpp"
#include "clock.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "window.hpp"

struct SDLApplication {
  bool running_{true};

  // for updateFpsCounter()
  double timeAccumulated_{0.0};
  int frameCount_{0};

  float u_offsetY_{0.0f};
  float u_offsetX_{0.0f};
  float u_offsetZ_{-3.0f}; // start at -3 for immediate visibility
  float u_rotate_{0.0f};
  float u_scale_{0.5f};

  Window window_;
  Shader shader_;
  Camera camera_;
  Mesh mesh_;
  Clock clock_;

  // infinite loop
  void run() {
    // start clock here to:
    // 1. exclude SDL and OPENGL initialization / setup time
    // 2. handle first call in loop
    clock_.start();
    while (running_) {
      advanceFrame();
    }
  }

  // easy to add breakpoint for debugging
  void advanceFrame() {
    const double frameTime = clock_.getFrameTime();
    handleInput();
    updateWorld();
    renderFrame();
    updateFpsCounter(frameTime);
  }

  // NOTE: use alpha interpolation?
  void renderFrame() {
    setup();
    draw();
    window_.swapBuffers();
  }

  // set up OPENGL state
  void setup() {
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClearColor(1.f, 1.f, 0.f, 1.f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    shader_.use();

    // ORDER OF OPERATIONS MATTERS !!!!

    // ------------------- LOCAL -> WORLD SPACE -----------------------
    glm::mat4 model = glm::translate(
        glm::mat4(1.0f), glm::vec3(u_offsetX_, u_offsetY_, u_offsetZ_));
    // Rotate object constantly
    u_rotate_ -= 1.0f;
    model = glm::rotate(
        model, glm::radians(u_rotate_), glm::vec3(0.0f, 1.0f, 0.0f));
    // Resize object
    model = glm::scale(model, glm::vec3(u_scale_, u_scale_, u_scale_));
    shader_.setMat4("u_modelMatrix", &model[0][0]);

    // ------------------- WORLD -> VIEW SPACE  -----------------------
    glm::mat4 view = camera_.getViewMatrix();
    shader_.setMat4("u_viewMatrix", &view[0][0]);

    // ------------------- VIEW -> CLIP SPACE -----------------------
    glm::mat4 perspective = glm::perspective(
        glm::radians(45.0f), // field of view (vertical), in radians
        static_cast<float>(window_.getWidth()) /
            static_cast<float>(
                window_.getHeight()), // aspect ratio (width / height)
        0.1f,   // near plane — anything closer gets clipped
        10.0f); // far plane — anything farther gets clipped
    shader_.setMat4("u_perspectiveProjection", &perspective[0][0]);
  }

  // OPENGL draw calls
  void draw() {
    mesh_.draw();
    // stop using current pipeline (unnecessary if 1 pipeline)
    shader_.unuse();
  }

  // display frames in window title
  void updateFpsCounter(const double frameTime) {
    timeAccumulated_ += frameTime;
    frameCount_++;
    if (timeAccumulated_ >= 1.0) {
      // frameCount_ / timeAccumulator_ rather than averaging 1/frameTime each
      // frame to give true average FPS over that second rather than an
      // average-of-instantaneous-rates
      double fps = frameCount_ / timeAccumulated_;
      timeAccumulated_ -= 1.0;
      frameCount_ = 0;
      window_.setTitle(
          ("MyWindow - FPS: " + std::to_string(static_cast<int>(fps))).c_str());
    }
  }

  // uses fixed-step (0 or more, depends on accumulatedTime_)
  void updateWorld() {
    while (clock_.shouldStep()) {
      // updateLogic/Physics/Anything(kFixedDt) HERE
      clock_.consumeStep();
    }
  }

  void processEvents() {
    // SDL_PollEvent() automatically calls SDL_PumpEvents() before checking
    // event queue which handles everything that happened since last frame
    SDL_Event event;
    // this loop drains discrete events (one time toggle)
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running_ = false;
      } else if (event.type == SDL_EVENT_MOUSE_MOTION &&
                 window_.isMouseCaptured()) {
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
  }

  // Uses ARROW keys
  void processObjectMovement(const bool* keys) {
    // X axis (left)
    if (keys[SDL_SCANCODE_LEFT]) {
      u_offsetX_ -= 0.01f;
    }
    // X axis (right)
    if (keys[SDL_SCANCODE_RIGHT]) {
      u_offsetX_ += 0.01f;
    }
    // Y axis (up)
    if (keys[SDL_SCANCODE_UP]) {
      u_offsetY_ += 0.01f;
    }
    // Y axis (down)
    if (keys[SDL_SCANCODE_DOWN]) {
      u_offsetY_ -= 0.01f;
    }
  }

  // Uses WASD keys
  void processCameraMovement(const bool* keys) {
    float speed = 0.1f; // NOTE: make better abstraction later.
    // Z axis (forward)
    if (keys[SDL_SCANCODE_W]) {
      camera_.moveForward(speed);
    }
    // Z axis (backward)
    if (keys[SDL_SCANCODE_S]) {
      camera_.moveBackward(speed);
    }
    // X axis (left)
    if (keys[SDL_SCANCODE_A]) {
      camera_.moveLeft(speed);
    }
    // X axis (right)
    if (keys[SDL_SCANCODE_D]) {
      camera_.moveRight(speed);
    }
  }

  // get and handle user input
  void handleInput() {
    processEvents();

    // SDL_GetKeyboardState() polls current state (gets snapshot every frame)
    const bool* keys = SDL_GetKeyboardState(nullptr);
    processObjectMovement(keys);
    processCameraMovement(keys);
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

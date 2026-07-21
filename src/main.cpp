#include <SDL3/SDL.h>
#include <stdexcept>
#include <string>

// ===================================================================
// ===================================================================
// ===================================================================

struct SDLApplication {
  // member variables
  SDL_Window* window_{nullptr};
  bool running_{true};

  // fps tracking
  double fpsTimer_{0.0};
  int frameCount_{0};

  // timing
  static constexpr double kMaxFrameTime{0.25};  // for clamping irregularities
  static constexpr double kFixedDt{1.0 / 60.0}; // for physics (in seconds)
  Uint64 frameStart_{0};
  double accumulatedTime_{0.0};

  // constructor
  SDLApplication() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
      SDL_Quit();
      throw std::runtime_error(std::string("SDL_Init failed: ") +
                               SDL_GetError());
    }

    window_ = SDL_CreateWindow("MyWindow",          // window title
                               800,                 // width in pixels
                               600,                 // height in pixels
                               SDL_WINDOW_RESIZABLE // flags go here
    );

    if (!window_) {
      SDL_Quit();
      throw std::runtime_error(std::string("Window creation failed: ") +
                               SDL_GetError());
    }
    // edge case, start clock here to exclude:
    // - SDL startup times
    // - handling special-case (first call in loop)
    frameStart_ = SDL_GetTicksNS();
  }

  // destructor
  ~SDLApplication() {
    SDL_DestroyWindow(window_);
    SDL_Quit();
  }

  // infinite loop
  void run() {
    while (running_) {
      advanceFrame();
    }
  }

  // easy to add breakpoint for debugging
  void advanceFrame() {
    double frameTime = getFrameTime();
    accumulatedTime_ += frameTime;
    // 1. get whatever user input to act on
    getInput();
    // 2. update using fixed-step (0 or more, depends on accumulatedTime_)
    updateWorld();
    // 3.
    // renderFrame(/*using alpha interpolation?*/);
    updateFpsCounter(frameTime);
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
      std::string title =
          "MyWindow - FPS: " + std::to_string(static_cast<int>(fps));
      SDL_SetWindowTitle(window_, title.c_str());

      fpsTimer_ = 0.0;
      frameCount_ = 0;
    }
  }

  void updateWorld() {
    while (accumulatedTime_ >= kFixedDt) {
      // explicit param for readability and future reusability
      // updateLogic/Physics/Anything(kFixedDt);
      accumulatedTime_ -= kFixedDt;
    }
  }

  double getFrameTime() {
    Uint64 frameEnd = SDL_GetTicksNS();
    double frameTime = (frameEnd - frameStart_) / 1e9; // convert to seconds
    frameStart_ = frameEnd;

    // clamp huge frame times for whatever reason
    if (frameTime > kMaxFrameTime) {
      frameTime = kMaxFrameTime;
    }
    return frameTime;
  }

  void getInput() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running_ = false;
      }
    }
  }
};

// ===================================================================
// ===================================================================
// ===================================================================

int main(int argc, char* argv[]) {
  try {
    SDLApplication app{};
    app.run();
  } catch (const std::exception& e) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error: %s", e.what());
    return 1;
  }
  return 0;
}

// ===================================================================
// ===================================================================
// ===================================================================

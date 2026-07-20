#include <SDL3/SDL.h>

struct SDLApplication {
  // member variables
  SDL_Window* window_{nullptr};
  bool running_{true};

  // constructor
  SDLApplication() {
    SDL_Init(SDL_INIT_VIDEO);
    window_ = SDL_CreateWindow("MyWindow",          // window title
                               800,                 // width in pixels
                               600,                 // height in pixels
                               SDL_WINDOW_RESIZABLE // flags go here
    );
    
    // verify window created succesfully
    if (window_ == nullptr) {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n",
                   SDL_GetError());
      running_ = false;
    }
  }

  // destructor
  ~SDLApplication() { SDL_Quit(); }

  // infinite loop
  void run() {
    while (running_) {
      advanceFrame();
    }
  }

  void advanceFrame() {
    getInput();
    updateLogic();
    renderFrame();
  }


  void getInput() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running_ = false;
      }
    }
  }

  void updateLogic() {}

  void renderFrame() {}

};

// ===================================================================
// ===================================================================
// ===================================================================

int main(int argc, char* argv[]) {
  SDLApplication app{};
  app.run();
  return 0;
}

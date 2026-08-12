#include "window.hpp"
#include <glad/glad.h>
#include <iostream>
#include <stdexcept>

  Window::Window() {
    setGLAttributes();
    createWindow();
    initializeGL();
  }

  Window::~Window() {
    SDL_GL_DestroyContext(glContext_);
    SDL_DestroyWindow(window_);
  }

  void Window::createWindow() {
    window_ = SDL_CreateWindow(title_.c_str(), width_, height_, flags_);

    if (!window_) {
      SDL_Quit();
      throw std::runtime_error(std::string("Window creation failed: ") +
                               SDL_GetError());
    }
    // locks and hides mouse in center for FPS-like relative movement
    SDL_SetWindowRelativeMouseMode(window_, true);
  }


  // Configures the OpenGL context attributes before creating the context.
  void Window::setGLAttributes() {
    // latest version supported on MACOS is 4.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    // use modern version of OPENGL, deprecated functions are disabled
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    // enable double buffering to avoid single buffer screen flickering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // request 24-bit depth buffer for depth testing
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  }

  void Window::initializeGL() {
    // create the OPENGL context and attach to SDL window
    // tells SDL_GL_SwapWindow() WHERE to display
    glContext_ = SDL_GL_CreateContext(window_);
    if (!glContext_) {
      SDL_DestroyWindow(window_);
      throw std::runtime_error(std::string("GL context creation failed: ") +
                               SDL_GetError());
    }

    // load all OPENGL function pointers via GLAD, using SDL to find them
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
      SDL_GL_DestroyContext(glContext_);
      SDL_DestroyWindow(window_);
      throw std::runtime_error("Failed to initialize GLAD");
    }

    // print OPENGL info and verify the OPENGL setup works as intended
    std::cout << "Vendor   : " << glGetString(GL_VENDOR) << '\n';
    std::cout << "Renderer : " << glGetString(GL_RENDERER) << '\n';
    std::cout << "Version  : " << glGetString(GL_VERSION) << '\n';
    std::cout << "GLSL     : " << glGetString(GL_SHADING_LANGUAGE_VERSION)
              << '\n';

    // setup vsync 1 = on, 0 = off, -1 = adaptive (falls back if unsupported)
    // "wait for the monitor's vertical blank before swapping buffers" — caps
    // your framerate at the monitor's refresh rate and prevents screen tearing
    SDL_GL_SetSwapInterval(1);
  }

  void Window::swapBuffers() { SDL_GL_SwapWindow(window_); }

  void Window::setTitle(const char* title) {
    SDL_SetWindowTitle(window_, title);
  }
  
  void Window::setMouseCaptured(bool captured) {
    mouseCaptured_ = captured;
    SDL_SetWindowRelativeMouseMode(window_, captured);
  }

  const bool Window::isMouseCaptured() const { return mouseCaptured_; }

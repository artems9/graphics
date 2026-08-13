#pragma once
#include <SDL3/SDL.h>
#include <string>

class Window {
  public:
    Window();
    ~Window();
    Window(const Window& other) = delete;
    Window& operator=(const Window& other) = delete;
    // deleting copy also makes moves deleted by consequence
    Window(Window&& other) = delete;
    Window& operator=(Window&& other) = delete;

    void swapBuffers();
    void resize(int width, int height);
    void setTitle(const char* title);
    void setMouseCaptured(bool captured);
    const bool isMouseCaptured() const;

  private:
    void createWindow();
    void setGLAttributes();
    void initializeGL();

    bool mouseCaptured_       {true};
    int width_                {800};
    int height_               {600};
    std::string title_        {"WINDOW TITLE HERE"};

    SDL_Window* window_       {nullptr};
    SDL_WindowFlags flags_    {SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL};
    SDL_GLContext glContext_  {nullptr};
};

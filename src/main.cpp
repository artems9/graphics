#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// ===================================================================
// ===================================================================
// ===================================================================

struct SDLApplication {
  // member variables
  SDL_Window* window_{nullptr};
  SDL_GLContext glContext_{nullptr};
  bool running_{true};

  // fps tracking
  double fpsTimer_{0.0};
  int frameCount_{0};

  // timing
  static constexpr double kMaxFrameTime{0.25};  // for clamping irregularities
  static constexpr double kFixedDt{1.0 / 60.0}; // for physics (in seconds)
  Uint64 frameStart_{0};
  double accumulatedTime_{0.0};

  // triangle
  GLuint vao_{0};
  GLuint vbo_{0};
  GLuint shaderProgram_{0};
  const std::string vertexShaderSource_{
      "#version 410 core\n"
      "in vec4 position;\n"
      "void main()\n"
      "{\n"
      "   gl_Position = vec4(position.x, position.y, position.z, position.w);\n"
      "}\n"};
  const std::string fragmentShaderSource_{
      "#version 410 core\n"
      "out vec4 color;\n"
      "void main()\n"
      "{\n"
      "   color = vec4(1.0f, 0.5f, 0.0f, 1.0f);\n"
      "}\n"};

  // constructor
  SDLApplication() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
      SDL_Quit();
      throw std::runtime_error(std::string("SDL_Init failed: ") +
                               SDL_GetError());
    }

    // set OPENGL context attributes
    // latest version supported on MACOS is 4.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    // uses modern version of OPENGL, deprecated functions are disabled
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    // enable double buffering to avoid single buffer screen flickering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // requests 24-bit depth buffer, storing how far away every pixel is
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    window_ = SDL_CreateWindow("MyWindow",          // window title
                               800,                 // width in pixels
                               600,                 // height in pixels
                               SDL_WINDOW_RESIZABLE // flags go here
                                   | SDL_WINDOW_OPENGL);

    if (!window_) {
      SDL_Quit();
      throw std::runtime_error(std::string("Window creation failed: ") +
                               SDL_GetError());
    }

    // attach OPENGL context to SDL window
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

    // verify the OPENGL setup works as intended
    std::cout << "Vendor   : " << glGetString(GL_VENDOR) << '\n';
    std::cout << "Renderer : " << glGetString(GL_RENDERER) << '\n';
    std::cout << "Version  : " << glGetString(GL_VERSION) << '\n';
    std::cout << "GLSL     : " << glGetString(GL_SHADING_LANGUAGE_VERSION)
              << '\n';

    // setup vsync 1 = on, 0 = off, -1 = adaptive (falls back if unsupported)
    SDL_GL_SetSwapInterval(1);

    setupVertices();

    setupPipeline();

    // start clock here to:
    // exclude SDL and OPENGL initialization and setup times
    // handle (edge case) first call in loop
    frameStart_ = SDL_GetTicksNS();
  }

  // destructor
  ~SDLApplication() {
    SDL_GL_DestroyContext(glContext_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
  }

  void setupPipeline() {
    shaderProgram_ =
        createShaderProgram(vertexShaderSource_, fragmentShaderSource_);
  }

  GLuint createShaderProgram(const std::string& vertexShaderSource,
                             const std::string& fragmentShaderSource) {
    GLuint programObj = glCreateProgram();

    GLuint vertexShader =
        getCompiledShader(GL_VERTEX_SHADER, vertexShaderSource);

    GLuint fragmentShader =
        getCompiledShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    glAttachShader(programObj, vertexShader);
    glAttachShader(programObj, fragmentShader);
    glLinkProgram(programObj);

    // validate program
    glValidateProgram(programObj);

    // glDetachShader, glDeleteShader

    return programObj;
  }

  GLuint getCompiledShader(GLuint type, const std::string& shaderSource) {
    GLuint shaderObj;

    if (type == GL_VERTEX_SHADER) {
      shaderObj = glCreateShader(GL_VERTEX_SHADER);
    } else if (type == GL_FRAGMENT_SHADER) {
      shaderObj = glCreateShader(GL_FRAGMENT_SHADER);
    }

    const char* src{shaderSource.c_str()};
    glShaderSource(shaderObj, 1, &src, nullptr);
    glCompileShader(shaderObj);

    return shaderObj;
  }

  void setupVertices() {
    // specify vertices as data for CPU
    // GLfloat is more compatible across different architectures
    const std::vector<GLfloat> vertexPosition{
        // x    y     z
        -0.8f, -0.8f, 0.0f, // vertex 1
        0.8f,  -0.8f, 0.0f, // vertex 2
        0.0f,  0.8f,  0.0f, // vertex 3
    };

    // SETUP / COPY DATA OVER TO GPU

    // generate x amount of VAO (instructions for reading the VBO)
    glGenVertexArrays(1, &vao_);

    // select VAO to use
    glBindVertexArray(vao_);

    // generate x amount of VBO (actual vertex data)
    glGenBuffers(1, &vbo_);

    // select VBO to use
    // GL_ARRAY_BUFFER can be thought of as:
    // A pointer (or handle) to the currently selected VBO.
    // so subsequent functions operating on GL_ARRAY_BUFFER refer to bound vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    // populate VBO with data
    glBufferData(GL_ARRAY_BUFFER,                         // target
                 vertexPosition.size() * sizeof(GLfloat), // size
                 vertexPosition.data(),                   // pointer to data
                 GL_STATIC_DRAW);                         // usage type

    // populate the currently bound VAO with instructions
    // by telling opengl how to interpret currently bound VBO
    glVertexAttribPointer(
        0,        // attribute index
        3,        // num. components per attribute
        GL_FLOAT, // type
        GL_FALSE, // data value normalization
        0,        // stride (byte offset between attributes)
        (void*)0  // offset into the bound VBO where this attribute begins
    );

    // Enable attribute 0 for input to the vertex shader.
    glEnableVertexAttribArray(0);

    // cleanup
    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
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
    renderFrame(/*using alpha interpolation?*/);
    // display frames in window title
    updateFpsCounter(frameTime);
  }

  void renderFrame() {
    preDraw();

    draw();

    // update screen (swap buffers)
    SDL_GL_SwapWindow(window_);
  }

  void preDraw() {

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // set up viewport
    // maps Normalized Device Coordinates into framebuffer size of window
    glViewport(0, 0, 800, 600);
    glClearColor(1.f, 1.f, 0.f, 1.f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram_);
  }

  void draw() {
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
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

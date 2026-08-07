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

struct SDLApplication {
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
  // Vertex Array Object (VAO) - How to interpret the data.
  // Stores the vertex attribute layout and the VBO/IBO bindings needed to
  // interpret vertex data.
  GLuint vao_{0};
  // Vertex Buffer Object (VBO) - What the data is.
  // Stores vertex data (positions, colors, UVs, etc.) in GPU memory.
  GLuint vbo_{0};
  // Index Buffer Object (IBO) - Which vertices to draw.
  // Stores indices that define how vertices are reused to form primitives.
  GLuint ibo_{0};
  GLuint shaderProgram_{0};

  float u_offsetY_{0.0f};
  float u_offsetX_{0.0f};
  float u_offsetZ_{-3.0f};  // start at -3 for immediate visibility

  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  // constructor
  SDLApplication() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
      SDL_Quit();
      throw std::runtime_error(std::string("SDL_Init failed: ") +
                               SDL_GetError());
    }

    setOpenglContextAttributes();

    createWindow();

    initOpengl();

    setupVertices();

    setupPipeline();

    // start clock here to:
    // exclude SDL and OPENGL initialization and setup times
    // handle (edge case) first call in loop
    frameStart_ = SDL_GetTicksNS();
  }

  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  // destructor
  ~SDLApplication() {
    SDL_GL_DestroyContext(glContext_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
  }

  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  void initOpengl() {
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
    SDL_GL_SetSwapInterval(1);
  }

  // SDL window creation
  void createWindow() {
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
  }

  // Configures the OpenGL context attributes before creating the context.
  void setOpenglContextAttributes() {
    // latest version supported on MACOS is 4.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    // uses modern version of OPENGL, deprecated functions are disabled
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // enable double buffering to avoid single buffer screen flickering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // request 24-bit depth buffer for depth testing
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  }

  void setupPipeline() {
    const std::string vertexShaderSource =
        getShaderSource("./shaders/vert.glsl");
    const std::string fragmentShaderSource =
        getShaderSource("./shaders/frag.glsl");

    shaderProgram_ =
        createShaderProgram(vertexShaderSource, fragmentShaderSource);
  }

  std::string getShaderSource(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
      throw std::runtime_error("Could not open shader file: " + path);
    }

    std::stringstream buffer;
    // get whole file into stream
    buffer << file.rdbuf();
    return buffer.str();
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

  void verifyCompilation(GLuint shaderObj) {
    GLint compilationResult;
    glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &compilationResult);

    if (compilationResult == GL_FALSE) {
      // find require buffer size for the info log
      GLint msgLength;
      glGetShaderiv(shaderObj, GL_INFO_LOG_LENGTH, &msgLength);

      // allocate buffer for outputting error message
      std::string errorMsg(msgLength, '\0');

      // Retrieve the shader compiler's error log.
      glGetShaderInfoLog(shaderObj, msgLength, nullptr, errorMsg.data());

      throw std::runtime_error("Shader compilation failed:\n" + errorMsg);

      // delete broken shader
      glDeleteShader(shaderObj);
    }
  }

  GLuint getCompiledShader(GLuint type, const std::string& shaderSource) {
    GLuint shaderObj;

    // Create shader object specifically for the type passed in
    if (type == GL_VERTEX_SHADER) {
      shaderObj = glCreateShader(GL_VERTEX_SHADER);
    } else if (type == GL_FRAGMENT_SHADER) {
      shaderObj = glCreateShader(GL_FRAGMENT_SHADER);
    }

    const char* src{shaderSource.c_str()};
    glShaderSource(shaderObj, 1, &src, nullptr);
    glCompileShader(shaderObj);

    verifyCompilation(shaderObj);

    return shaderObj;
  }

  void setupVertices() {

    // OPENGL uses counter clockwise winding order by default
    // rendering QUAD
    const std::vector<GLfloat> vertexData{
        // clang-format off
        // Vertex 0 (bottom left)
        -0.5f, -0.5f, 0.0f, // position x, y, z
        1.0f, 0.0f, 0.0f,   // color    r, b, b
        // Vertex 1 (bottom right)
        0.5f, -0.5f, 0.0f, // position
        0.0f, 1.0f, 0.0f, // color
        // Vertex 2 (top right)
        0.5f, 0.5f, 0.0f, // position
        0.0f, 0.0f, 1.0f, // color
        // Vertex 3 (top left)
        -0.5f, 0.5f, 0.0f, // position
        1.0f, 0.0f, 1.0f, // color
        // clang-format on
    };

    // generate x amount of VAO (instructions for reading the VBO)
    glGenVertexArrays(1, &vao_);

    // select VAO to use
    glBindVertexArray(vao_);

    //===================/ SETUP VBO /===================
    // generate x amount of VBO (actual vertex data)
    glGenBuffers(1, &vbo_);

    // select VBO to use
    // GL_ARRAY_BUFFER can be thought of as:
    // A pointer (or handle) to the currently selected VBO.
    // so subsequent functions operating on GL_ARRAY_BUFFER refer to bound vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    // populate target buffer used by GPU, with data from VBO (vertexPositions)
    glBufferData(GL_ARRAY_BUFFER,                     // target buffer
                 vertexData.size() * sizeof(GLfloat), // size in bytes
                 vertexData.data(),                   // pointer to data
                 GL_STATIC_DRAW);                     // usage type

    const std::vector<GLuint> iboData{0, 1, 3, 1, 2, 3};

    // setup IBO (Index Buffer Object) or EBO (Element Buffer Object)
    glGenBuffers(1, &ibo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    // populate IBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 iboData.size() * sizeof(GLuint),
                 iboData.data(),
                 GL_STATIC_DRAW);

    // ================== SETUP VAO POSITION =====================
    // populate the currently bound VAO with instructions
    // by telling opengl how to interpret currently bound VBO
    glVertexAttribPointer(
        0,        // attribute index, corresponds to glEnableVertexAttribArray
        3,        // num. components per attribute (x, y, z)
        GL_FLOAT, // type
        GL_FALSE, // data value normalization
        sizeof(GLfloat) * 6, // stride (byte offset between attribute)
        nullptr // offset into the bound VBO where this attribute begins
    );
    // Enable attributes for input to the vertex shader.
    glEnableVertexAttribArray(0);

    // ================== SETUP VAO COLOR =====================
    glVertexAttribPointer(
        1,        // attribute index, corresponds to glEnableVertexAttribArray
        3,        // num. components per attribute (r, g, b)
        GL_FLOAT, // type
        GL_FALSE, // data value normalization
        sizeof(GLfloat) * 6, // stride (byte offset between attribute)
        reinterpret_cast<const void*>(
            sizeof(GLfloat) *
            3) // offset into the bound VBO where this attribute begins
    );
    // Enable attribute for input to the vertex shader.
    glEnableVertexAttribArray(1);

    // Unbind currently bound VAO
    glBindVertexArray(0);
    // Disable any attributes previously enabled in VAO
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
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
    SDL_GL_SwapWindow(window_);
  }

  // set up OPENGL state
  void preDraw() {
    // Disable depth test and face culling
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // set up viewport
    // maps Normalized Device Coordinates into framebuffer size of window
    glViewport(0, 0, 800, 600);

    // Initialize clear color (background of screen)
    glClearColor(1.f, 1.f, 0.f, 1.f);

    // Clear color and depth buffers
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Use shader program
    glUseProgram(shaderProgram_);


    // ------------------- TRANSLATION -----------------------

    // Model transformation by translating our object into world space
    glm::mat4 translate = glm::translate(
        glm::mat4(1.0f), glm::vec3(u_offsetX_, u_offsetY_, u_offsetZ_));

    // uniform vairable name must match the one declared across shaders
    GLint uniformLocation =
        glGetUniformLocation(shaderProgram_, "u_modelMatrix");

    if (uniformLocation >= 0) {
      glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &translate[0][0]);
    } else {
      std::println("Could not find u_modelMatrix location");
      exit(EXIT_FAILURE);
    }

    // ------------------- PROJECTION -----------------------

    // Projection matrix (in perspective)
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f),
                                             800.0f / 400.0f,
                                             0.1f,   // how close we can see
                                             10.0f); // how far we can see

    // Retrieve location of perspective matrix uniform
    GLint uPerspectiveLocation =
        glGetUniformLocation(shaderProgram_, "u_perspectiveProjection");

    if (uPerspectiveLocation >= 0) {
      glUniformMatrix4fv(uPerspectiveLocation, 1, GL_FALSE, &perspective[0][0]);
    } else {
      std::println("Could not find u_pespectiveProjection location");
      exit(EXIT_FAILURE);
    }
  }

  // OPENGL draw calls
  void draw() {
    // enable attributes
    glBindVertexArray(vao_);
    // select VBO we want to enable
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    // render indexed data
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    // stop using current pipeline, unnecessary if we only have 1 pipleine
    glUseProgram(0);
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

    // SDL_PollEvent() automatically calls SDL_PumpEvents() before checking
    // event queue, handles everything that happened since last frame
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running_ = false;
      }
    }
    // tells you the CURRENT STATE of the keyboard (every frame)
    const bool* keys = SDL_GetKeyboardState(nullptr);

    // X axis (left/right)
    if (keys[SDL_SCANCODE_LEFT] == true) {
      u_offsetX_ -= 0.01f;
    }
    if (keys[SDL_SCANCODE_RIGHT] == true) {
      u_offsetX_ += 0.01f;
    }

    // Y axis (up/down)
    if (keys[SDL_SCANCODE_UP] == true) {
      u_offsetY_ += 0.01f;
    }
    if (keys[SDL_SCANCODE_DOWN] == true) {
      u_offsetY_ -= 0.01f;
    }


    // Z axis (forward/back - "in and out" from camera)
    if (keys[SDL_SCANCODE_W] == true) {
      u_offsetZ_ -= 0.01f; // more negative = further from camera
    }
    if (keys[SDL_SCANCODE_S] == true) {
      u_offsetZ_ += 0.01f; // more positive  = closer to camera
    }

  }
};

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

int main(int argc, char* argv[]) {
  try {
    SDLApplication app{};
    app.run();
  } catch (const std::exception& e) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error: %s", e.what());
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

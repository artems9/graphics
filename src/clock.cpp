#include "clock.hpp"

void Clock::start() { frameStart_ = SDL_GetTicksNS(); }

double Clock::getFrameTime() {
  Uint64 frameEnd = SDL_GetTicksNS();
  double frameTime = (frameEnd - frameStart_) / 1e9; // convert to seconds
  frameStart_ = frameEnd;

  // clamp huge frame times for whatever reason
  if (frameTime > kMaxFrameTime) {
    frameTime = kMaxFrameTime;
  }

  accumulatedTime_ += frameTime;

  return frameTime;
}

bool Clock::shouldStep() const { return accumulatedTime_ >= kFixedDt; }

void Clock::consumeStep() { accumulatedTime_ -= kFixedDt; }



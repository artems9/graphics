#pragma once
#include <SDL3/SDL.h>

class Clock {
public:
  void start();
  double getFrameTime();
  bool shouldStep() const;
  void consumeStep(); 

private:
  static constexpr double kMaxFrameTime     {0.25};
  static constexpr double kFixedDt          {1.0 / 60.0};

  Uint64 frameStart_                        {0};
  double accumulatedTime_                   {0.0};
};

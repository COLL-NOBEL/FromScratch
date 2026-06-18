#pragma once
// engine/core/Time.hpp
// Frame timing and delta-time tracking.

#include <chrono>

namespace engine {

class Time {
public:
    Time();

    // Call at the start of every frame
    void Tick();

    // Seconds since last frame
    float GetDeltaTime()  const { return deltaTime; }

    // Seconds since engine started
    float GetTotalTime()  const { return totalTime; }

    // Frames per second (smoothed)
    float GetFPS()        const { return fps; }

    // Raw frame count
    uint64_t GetFrameCount() const { return frameCount; }

    // Clamp delta-time to avoid spiral of death (default: 0.1 s)
    void SetMaxDeltaTime(float maxDt) { maxDeltaTime = maxDt; }

private:
    using Clock     = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    TimePoint startTime;
    TimePoint lastTime;

    float    deltaTime    = 0.0f;
    float    totalTime    = 0.0f;
    float    fps          = 0.0f;
    float    maxDeltaTime = 0.1f;
    uint64_t frameCount   = 0;

    // FPS smoothing
    float fpsAccum  = 0.0f;
    int   fpsFrames = 0;
};

} // namespace engine

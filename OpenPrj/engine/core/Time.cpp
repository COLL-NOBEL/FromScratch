// engine/core/Time.cpp

#include "Time.hpp"
#include <algorithm>

namespace engine {

Time::Time() {
    startTime = Clock::now();
    lastTime  = startTime;
}

void Time::Tick() {
    TimePoint now = Clock::now();

    using FloatSec = std::chrono::duration<float>;
    float raw = FloatSec(now - lastTime).count();
    lastTime  = now;

    deltaTime  = std::min(raw, maxDeltaTime);
    totalTime  = FloatSec(now - startTime).count();
    frameCount++;

    // Smooth FPS over ~0.5 s
    fpsAccum  += raw;
    fpsFrames++;
    if (fpsAccum >= 0.5f) {
        fps       = (float)fpsFrames / fpsAccum;
        fpsAccum  = 0.0f;
        fpsFrames = 0;
    }
}

} // namespace engine

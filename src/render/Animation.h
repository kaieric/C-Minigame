#pragma once

#include <algorithm>
#include <cmath>

namespace game::animation {
inline constexpr double framesPerSecond = 10.0;

inline int loopingFrame(double seconds, int frameCount)
{
    if (frameCount <= 1) return 0;
    const double period = frameCount / framesPerSecond;
    return std::min(frameCount - 1,
        static_cast<int>(std::fmod(seconds, period) * framesPerSecond));
}

inline int pingPongFrame(double seconds, int frameCount)
{
    if (frameCount <= 1) return 0;
    // 13 frames have 12 transitions per leg: 1.2 seconds at an average 10/s.
    const double legSeconds = (frameCount - 1) / framesPerSecond;
    const double phase = std::fmod(seconds, 2.0 * legSeconds) / legSeconds;
    const double progress = phase <= 1.0 ? phase : 2.0 - phase;
    // Constant acceleration, then constant deceleration. Position and velocity
    // are continuous at the midpoint; velocity is zero at each turnaround.
    const double eased = progress < 0.5
        ? 2.0 * progress * progress
        : 1.0 - 2.0 * (1.0 - progress) * (1.0 - progress);
    // Nearest frame treats both turnaround frames symmetrically.
    // Tiny tolerance makes exact half-frame boundaries round consistently
    // on both legs despite floating-point arithmetic.
    return std::clamp(static_cast<int>(std::lround(eased * (frameCount - 1) + 1e-9)),
                      0, frameCount - 1);
}
}

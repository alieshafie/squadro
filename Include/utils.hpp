#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <iostream>

namespace utils
{

    // Timing helpers
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    inline double elapsedSeconds(TimePoint start, TimePoint end)
    {
        return std::chrono::duration<double>(end - start).count();
    }

    // Clamp helper (for safety in bounds)
    template <typename T>
    T clamp(T val, T lo, T hi)
    {
        return (val < lo) ? lo : ((val > hi) ? hi : val);
    }

    // Debug/logging helper
    inline void log(const std::string &msg)
    {
        std::cerr << "[LOG] " << msg << std::endl;
    }

}

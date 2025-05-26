#pragma once
#include <string>
#include <chrono>
#include <iostream>

namespace utils
{
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

    inline double elapsedSeconds(const TimePoint &s, const TimePoint &e) { return std::chrono::duration<double>(e - s).count(); }

    inline void log(const std::string &msg) { std::cerr << "[LOG] " << msg << std::endl; }
}
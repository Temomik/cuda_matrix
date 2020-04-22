#ifndef TIME_HADLER
#define TIME_HADLER

#include <chrono>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;

enum ClockType
{
    cpu,
    gpu
};

enum TimeType
{
    milliseconds = 1000,
    seconds = 1
};

class Time
{
private:
    high_resolution_clock::time_point buffer;
    double time;
public:
    Time() = default;
    void start(ClockType type);
    void stop(ClockType type);
    double getElapsed(TimeType type) const;
    ~Time() = default;
};

#endif 
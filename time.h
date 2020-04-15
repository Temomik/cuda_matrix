#ifndef TIME_HADLER
#define TIME_HADLER

#include <chrono>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;

enum clockType
{
    cpu,
    gpu
};

class Time
{
private:
    high_resolution_clock::time_point buffer;
    double time;
public:
    Time() = default;
    void start(clockType type);
    void stop(clockType type);
    double get() const;
    ~Time() = default;
};

#endif 
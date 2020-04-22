#include "time.h"


void Time::start(ClockType type)
{
    if(type == ClockType::cpu)
    {
        buffer = high_resolution_clock::now();
    }
}

void Time::stop(ClockType type)
{
    if(type == ClockType::cpu)
    {
        time = duration_cast<duration<double> >(high_resolution_clock::now() - buffer).count();
    }
}

double Time::getElapsed(TimeType type) const
{
    return time*type;
}
#include "time.h"


void Time::start(clockType type)
{
    if(type == clockType::cpu)
    {
        buffer = high_resolution_clock::now();
    }
}

void Time::stop(clockType type)
{
    if(type == clockType::cpu)
    {
        time = duration_cast< duration<double> >(high_resolution_clock::now() - buffer).count();
    }
}

double Time::get() const
{
    return time;
}
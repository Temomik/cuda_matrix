#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>

#include "image_handler.h"
#include "gaus_matrix.h"
#include "time.h"
#define MAX_LENGTH 250
#define TAG 0

int main(void)
{
    ImageHandler image("7275.jpg");
    double time = 0;
    Time handler;
    handler.start(ClockType::cpu);
    image.gausFilterGpu(3,time);
    handler.stop(ClockType::cpu);
    std::cout << handler.getElapsed(TimeType::milliseconds) << "image" << std::endl;
    image.save("72751.jpg");
    return 0;
}
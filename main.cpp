#include <stdint.h>
#include <iostream>
#include "image_handler.h"
#include "gaus_matrix.h"
#include "time.h"

using std::cout;
using std::endl;


int main() 
{
    // Time handler;
    ImageHandler image("test.jpg");
    image.grayConvert();
    // image.concatImage(27/2,35/2);
    // handler.start(clockType::cpu);
    image.gausFilterGpu(3,handlerType::gray);
    // image.gausFilterGpu(3);
    // image.grayGausFilterCpu(100);
    // image.gausFilterCpu(3,handlerType::gray);
    // handler.stop(clockType::cpu);
    // cout << handler.get();
    image.save("copy.jpg");
    return 0;
}
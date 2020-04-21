#include <stdint.h>
#include <iostream>
#include "image_handler.h"
#include "gaus_matrix.h"
#include "time.h"

using std::cout;
using std::endl;


int main() 
{
    {
        ImageHandler image("test.jpg");
        image.grayConvert();
        image.gausFilterGpu(3,handlerType::gray);
        image.save("gpu.jpg");
    }
    // {
    //     ImageHandler image("test.jpg");
    //     image.grayConvert();
    //     image.gausFilterCpu(3,handlerType::gray);
    //     image.save("cpu.jpg");
    // }
    return 0;
}
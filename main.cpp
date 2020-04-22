#include <stdint.h>
#include <iostream>
#include "image_handler.h"
#include "gaus_matrix.h"
#include "time.h"

using std::cout;
using std::endl;

#define INPUT_FILENAME "resized.jpg"

int main() 
{
    
    {
        ImageHandler image(INPUT_FILENAME);
        // image.grayConvert();
        image.gausFilterGpu(4);
        image.save("gpu.jpg");
    }
    {
        ImageHandler image(INPUT_FILENAME);
        // image.grayConvert();
        image.gausFilterCpu(4);
        image.save("cpu.jpg");
    }
    return 0;
}
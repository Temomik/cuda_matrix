#include <stdint.h>
#include <iostream>
#include "image_handler.h"
#include "gaus_matrix.h"
#include "time.h"

using std::cout;
using std::endl;


int main() 
{
    Time handler;
    ImageHandler image("out.jpg");
    // image.gausFilterGpu(1);
    // image.grayConvert();
    // image.concatImage(45,50);
    handler.start(clockType::cpu);
    image.gausFilterCpu(3);
    handler.stop(clockType::cpu);
    cout << handler.get();
    image.save("copy.jpg");
    return 0;
}
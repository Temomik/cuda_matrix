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
    ImageHandler imageCpu(INPUT_FILENAME);
    ImageHandler imageGpu(INPUT_FILENAME);
    // image.grayConvert();
    // image.grayConvert();
    imageGpu.gausFilterGpu(3);
    imageCpu.gausFilterCpu(3);
    if(!imageCpu.compare(imageGpu))
    {
        cout << "(";
        return 1;
    }
    cout << "ok";
    return 0;
}
#include <stdint.h>
#include <iostream>
#include "image_handler.h"
#include "gaus_matrix.h"
using std::cout;
using std::endl;

#define CHANNEL_NUM 3

int main() 
{
    ImageHandler image("test.jpg");
    // image.grayConvert();
    image.gausFilter(5);
    image.save("out.png");
    return 0;
}
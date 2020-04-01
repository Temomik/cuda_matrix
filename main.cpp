#include <stdint.h>
#include <iostream>
#include "image_handler.h"
#include "gaus_matrix.h"
using std::cout;
using std::endl;

int main() 
{
    ImageHandler image("test.jpg");
    // image.grayConvert();
    image.gausFilter(150);
    image.save("out.png");
    return 0;
}
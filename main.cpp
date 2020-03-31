#include <stdint.h>
#include <iostream>
#include "image_handler.h"

using std::cout;
using std::endl;

#define CHANNEL_NUM 3

int main() 
{
    ImageHandler image("test.jpg");
    image.save("out.png");
    return 0;
}
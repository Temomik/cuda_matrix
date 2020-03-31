#include "image_handler.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "sub/stb_image_write.h"
#include "sub/stb_image.h"

ImageHandler::ImageHandler(string fileName,int32_t channelNum)
{
    load(fileName);
}

ImageHandler::~ImageHandler()
{
    stbi_image_free(pixels);
}

void ImageHandler::load(string fileName, int32_t channelNum)
{
    pixels = stbi_load(fileName.c_str(), &width, &height, &numComponents, channelNum);
    if(!pixels)
        throw std::runtime_error("Fail to load image");
}

void ImageHandler::save(string outFileName) const
{
    if(!pixels)
        throw std::runtime_error("Pixels is NULL");
    stbi_write_png(outFileName.c_str(), width, height, numComponents, pixels, width*numComponents);
}

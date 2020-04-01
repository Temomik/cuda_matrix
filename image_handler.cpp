#include "image_handler.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "sub/stb_image_write.h"
#include "sub/stb_image.h"
#include <stdexcept>
#include "gaus_matrix.h"
#include <vector>
#include <sstream>
#include <iomanip>

ImageHandler::ImageHandler(string fileName, int32_t channelNum)
    :fileName(fileName)
{
    load(fileName,channelNum);
}

ImageHandler::~ImageHandler()
{
    stbi_image_free(pixels);
}

void ImageHandler::load(string fileName, int32_t channelNum)
{
    pixels = stbi_load(fileName.c_str(), &width, &height, &numComponents, channelNum);
    if (!pixels)
        throw std::runtime_error("Fail to load image");
}

void ImageHandler::grayConvert()
{
    int64_t length = width * height * numComponents;
    for (size_t i = 0; i < length; i += 3)
    {
        int32_t buff = 0;
        for (size_t j = 0; j < 3; j++)
        {
            buff += pixels[i + j];
        }
        buff /= 3;
        for (size_t j = 0; j < 3; j++)
        {
            pixels[i + j] = buff;
        }
    }
}

void ImageHandler::save(string outFileName) const
{
    if (!pixels)
        throw std::runtime_error("Pixels is NULL");
    stbi_write_png(outFileName.c_str(), width, height, numComponents, pixels, width * numComponents);
}

uint8_t ImageHandler::getGrayElement(const int32_t it) const
{
    if (it < width * height)
        return pixels[it * 3];
    else
        return 0;
}


void ImageHandler::gausFilter(const int32_t size)
{
    vector<uint8_t> tmpPixels = getMatrix();
    if(size <= 1)
        return;
    GausMatrix matrix(size);
    int64_t maxSize = width * height * 3;
    for (int32_t i = 0; i < height; i++)
    {
        for (int32_t j = 0; j < width * 3; j += 3)
        {
            for (int32_t k = 0; k < 3; k++)
            {
                float buff = 0;
                for (int32_t p = 0; p < size; p++)
                {
                    for (int32_t m = 0; m < size; m++)
                    {
                        int32_t tmpInd = (i + p) * 3 * width + j + 3 * (m - size / 2) + k;
                        if(i + p - size / 2 >= 0)
                        if (tmpInd >= 0 && tmpInd < maxSize && tmpInd / (width*3) == i+p)
                        {
                            buff += tmpPixels[tmpInd] * matrix[p * size + m];
                        }
                    }
                }
                if(buff <= 255)
                    pixels[i * width * 3 + j + k] = buff;
            }
        }
    }
}

vector<uint8_t> ImageHandler::getMatrix() const
{
    vector<uint8_t> result;
    int64_t len = height * width * numComponents;
    result.reserve(len);
    for (size_t i = 0; i < len; i++)
    {
        result[i] = pixels[i];
    }
    return result;
}

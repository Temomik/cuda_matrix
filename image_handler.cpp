#include "image_handler.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "sub/stb_image_write.h"
#include "sub/stb_image.h"
#include <stdexcept>
#include "gaus_matrix.h"
#include <vector>
#include <iostream>

#ifdef __linux__
// #include "cuda_runtime.h"
#endif

ImageHandler::ImageHandler(string fileName, int32_t channelNum)
    : fileName(fileName)
{
    load(fileName, channelNum);
    grayArray = new uint8_t[width * height];
}

ImageHandler::~ImageHandler()
{
    delete[] grayArray;
    delete[] pixels;
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
    fillGrayArray();
}

void ImageHandler::save(string outFileName) const
{
    if (!pixels)
        throw std::runtime_error("Pixels is NULL");
    stbi_write_jpg(outFileName.c_str(), width, height, numComponents, pixels, width * numComponents);
}

uint8_t ImageHandler::getGrayElement(int32_t it) const
{
    if (it < width * height)
        return pixels[it * 3];
    else
        return 0;
}

void ImageHandler::gausFilterCpu(int32_t size)
{
    vector<uint8_t> tmpPixels = getMatrix();
    if (size <= 1)
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
                        int32_t tmpI = i + p - size / 2,
                                tmpJ = j + 3 * (m - size / 2) + k;
                        int32_t tmpInd = tmpI * 3 * width + tmpJ;
                        if (tmpInd >= 0 && tmpInd < maxSize && tmpInd / (width * 3) == tmpI)
                        {
                            buff += tmpPixels[tmpInd] * matrix[p * size + m];
                        }
                    }
                }
                if (buff <= 255 && buff >= 0)
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

namespace
{

#ifdef __linux__
__global__ void gausFilter(uint8_t *src, uint8_t *dst, int width, int height)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    printf("%d %d\n", x, y);
}
#endif

} // namespace

void ImageHandler::gausFilterGpu(int32_t size)
{
#ifdef __linux__
    uint8_t *dev_padded, *dev_result;
    // cudaMalloc((void **)&dev_padded, (width + 2) * (height + 2));
    // cudaMalloc((void **)&dev_result, width * height);
    // cudaMemcpy(dev_padded, padded, (width + 2) * (height + 2), cudaMemcpyHostToDevice);
    dim3 block(32, 32),
        grid(1, 2);

    // cudaEvent_t startTime;
    // cudaEvent_t stopTime;
    // cudaEventCreate(&startTime);
    // cudaEventCreate(&stopTime);
    // cudaEventRecord(startTime);

    gausFilter<<<grid, block>>>(dev_result, dev_padded, 0, 0);
    cudaDeviceSynchronize();

    // cudaEventRecord(stopTime);
    // cudaEventSynchronize(stopTime);
    // float resultTime;
    // cudaEventElapsedTime(&resultTime, startTime, stopTime);
    // printf("GPU time: %f miliseconds\n", resultTime);
    // cudaFree(dev_padded);
    // uint8_t *result = new uint8_t[width * height];
    // cudaMemcpy(result, dev_result, width * height, cudaMemcpyDeviceToHost);
    // cudaFree(dev_result);
#endif
}

void ImageHandler::fillGrayArray()
{
    size_t length = height * width;
    for (size_t i = 0; i < length; i += numComponents)
    {
        grayArray[i] = pixels[i];
    }
    std::cout << sizeof(grayArray) << std::endl;
}

void ImageHandler::concatImage(uint32_t x, uint32_t y)
{
    uint32_t newlength = height * width * numComponents * x * y;
    uint8_t *newPixels = new uint8_t[newlength];
    uint32_t baseXScale = width * 3,
             xScale = baseXScale * x,
             yScale = height * y;
    for (uint32_t i = 0; i < yScale; i += height)
    {
        for (uint32_t j = 0; j < xScale; j += baseXScale)
        {
            for (uint32_t k = 0; k < height; k++)
            {
                for (uint32_t m = 0; m < baseXScale; m += numComponents)
                {
                    for (uint32_t p = 0; p < numComponents; p++)
                    {
                        uint32_t tmpNewCord = i * xScale + j + k * xScale + m + p;
                        if (tmpNewCord >= 0 && tmpNewCord < newlength)
                        {
                            newPixels[tmpNewCord] = pixels[k *baseXScale + m + p];
                        }
                    }
                }
            }
        }
    }
    delete[] pixels;
    pixels = newPixels;
    width *= x;
    height *= y;
}

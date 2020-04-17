#include "image_handler.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "sub/stb_image_write.h"
#include "sub/stb_image.h"
#include <stdexcept>
#include "gaus_matrix.h"
#include <vector>
#include <iostream>
#include "time.h"
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
    int32_t maxSize = width * height * 3;
    uint8_t *tmpPixels = new uint8_t[maxSize];

    if (size <= 1)
        return;
    GausMatrix matrix(size);
    int32_t twoSize = size * size;
    int32_t halfSize = size / 2;
    Time handler;
    std::cout << " - start" << std::endl;
    handler.start(clockType::cpu);
    int32_t xScale = width * 3;
    double tmpCell = 0;

    for (int32_t i = 0; i < maxSize; i++)
    {
        for (int16_t p = 0; p < twoSize; p++)
        {
            int16_t x = p % size - halfSize;
            int16_t y = p / size - halfSize;
            int32_t tmpMatrixIndex = i + y * xScale + x * numComponents;
            if (tmpMatrixIndex >= 0 && tmpMatrixIndex < maxSize)
                tmpCell += matrix[p] * pixels[tmpMatrixIndex];
        }
        // if(tmpCell > 255)
        //     tmpCell = 255;
        tmpPixels[i] = static_cast<uint8_t>(tmpCell);
    }

    handler.stop(clockType::cpu);
    std::cout << handler.get() << " - stop" << std::endl;
    delete[] pixels;
    pixels = tmpPixels;
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

#ifdef __linux__
namespace
{
    int32_t customCeil(double num,int32_t del)
    {
        if(static_cast<int32_t>(num) % del > 0)
        {
            num /= del;
            num++;
        } else
        {
            num /= del;
        }
        return num;
    }

__global__ void gausFilter(uint8_t *inBuffer, uint8_t *outBuffer, uint32_t width, uint32_t height, double *gausMatrix, uint32_t matrixSize)
{
    int32_t x = blockIdx.x * blockDim.x + threadIdx.x;
    int32_t y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x < width && y < height)
    {
        int32_t cord = y*width+x;
        int32_t halfMatrixSize = matrixSize /  2;
        for (int32_t i = 0; i < matrixSize; i++)
        {
            for (int32_t j = 0; j < matrixSize; j++)
            {
                int32_t tmpCord = cord + width * (i - halfMatrixSize) + (j - halfMatrixSize)*3;
                if(tmpCord >= 0 && tmpCord < width*height)
                {
                    // if(i == 0 && j == 0)
                    // printf("%f\n",gausMatrix[0]);
                    // outBuffer[cord] = inBuffer[cord];
                    outBuffer[cord] += gausMatrix[i*matrixSize+j]*inBuffer[tmpCord];
                }
            }
        }
                
        // for (int16_t p = 0; p < twoSize; p++)
        // {
        //     int16_t x = p % size - halfSize;
        //     int16_t y = p / size - halfSize;
        //     int32_t tmpMatrixIndex = i + y * xScale + x * numComponents;
        //     if (tmpMatrixIndex >= 0 && tmpMatrixIndex < maxSize)
        //         tmpPixels[i] += matrix[p] * pixels[tmpMatrixIndex];
        // }
        // outBuffer[cord] = inBuffer[cord];
    }
}
} // namespace
#endif

#ifdef __linux__
void ImageHandler::gausFilterGpu(int32_t size)
{
    int32_t bufferSize = width * height * numComponents;
    uint8_t *cudaOutBuffer,
        *cudaInBuffer;
    double *gausMatrix;
    GausMatrix matrix(size);

    if (cudaMalloc(&cudaOutBuffer, bufferSize) != cudaSuccess)
    {
        std::runtime_error("Fail allocate gpu memory to cudaOutBuffer");
    }

    if (cudaMalloc(&cudaInBuffer, bufferSize) != cudaSuccess)
    {
        cudaFree(cudaOutBuffer);
        std::runtime_error("Fail allocate gpu memory to cudaInBuffer");
    }

    if (cudaMalloc(&gausMatrix, size*size*sizeof(double)) != cudaSuccess)
    {
        cudaFree(cudaOutBuffer);
        cudaFree(cudaInBuffer);
        std::runtime_error("Fail allocate gpu memory to gausMatrix");
    }
	cudaMemcpy(cudaInBuffer, pixels, bufferSize, cudaMemcpyHostToDevice);
	cudaMemcpy(gausMatrix, matrix.getMatrix(), size*size*sizeof(double), cudaMemcpyHostToDevice);
    
    dim3 block(32, 32),
        grid(customCeil(width * numComponents, 32), customCeil(height,32));
    cudaEvent_t startTime;
    cudaEvent_t stopTime;
    cudaEventCreate(&startTime);
    cudaEventCreate(&stopTime);
    cudaEventRecord(startTime);
    gausFilter<<<grid, block>>>(cudaInBuffer, cudaOutBuffer, width * numComponents, height, gausMatrix, size);
    cudaDeviceSynchronize();
    cudaEventSynchronize(stopTime);
    cudaEventRecord(stopTime);
    float resultTime;
    cudaEventElapsedTime(&resultTime, startTime, stopTime);
    printf("GPU time: %f miliseconds\n", resultTime);
	cudaMemcpy(pixels, cudaOutBuffer, bufferSize, cudaMemcpyDeviceToHost);
    cudaFree(cudaOutBuffer);
    cudaFree(cudaInBuffer);
    cudaFree(gausMatrix);
}
#endif

void ImageHandler::fillGrayArray()
{
    size_t length = height * width;
    for (size_t i = 0; i < length; i += numComponents)
    {
        grayArray[i] = pixels[i];
    }
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
                            newPixels[tmpNewCord] = pixels[k * baseXScale + m + p];
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

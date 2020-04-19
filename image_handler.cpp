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
#include "cuda_runtime.h"
#endif

ImageHandler::ImageHandler(string fileName, int64_t channelNum)
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

void ImageHandler::load(string fileName, int64_t channelNum)
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
        int64_t buff = 0;
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

uint8_t ImageHandler::getGrayOneComponent(int64_t it,int64_t num) const
{
    if (it < width * height)
        return pixels[it * num];
    else
        return 0;
}

void ImageHandler::gausFilterCpu(int64_t size,handlerType type)
{
    int64_t maxSize = width * height;
    uint8_t* switchPtr = grayArray;
    if(type == handlerType::rgb)
    {
        switchPtr = pixels;
        maxSize *= numComponents;
    }

    uint8_t *tmpPixels = new uint8_t[maxSize];
    if (size <= 1)
        return;
    
    GausMatrix matrix(size);
    int64_t twoSize = size * size;
    int64_t halfSize = size / 2;
    Time handler;
    std::cout << " - start" << std::endl;
    handler.start(clockType::cpu);
    int64_t xScale = width * 3;

    for (int64_t i = 0; i < maxSize; i++)
    {
        double tmpCell = 0;
        for (int16_t p = 0; p < twoSize; p++)
        {
            int16_t x = p % size - halfSize;
            int16_t y = p / size - halfSize;
            int64_t tmpMatrixIndex = i + y * xScale + x * numComponents;
            if (tmpMatrixIndex >= 0 && tmpMatrixIndex < maxSize)
                tmpCell += matrix[p] * switchPtr[tmpMatrixIndex];
        }
        if(tmpCell > 255)
            tmpCell = 255;
        tmpPixels[i] = static_cast<uint8_t>(tmpCell);
    }

    handler.stop(clockType::cpu);
    std::cout << handler.get() << " - stop" << std::endl;

    if(type == handlerType::rgb)
    {
        delete[] pixels;
        pixels = tmpPixels;
        return;
    }
    delete[] grayArray;
    grayArray = tmpPixels;
    concatGrayComponents();
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

void ImageHandler::concatGrayComponents()
{
    int64_t length = width * height;
    for (uint64_t i = 0; i < length; i++)
    {
        for (uint64_t j = 0; j < numComponents && (j+i*numComponents) < length*numComponents; j++)
        {
            pixels[i*numComponents+j] = grayArray[i];
        }
    }
}

#ifdef __linux__
namespace
{
    int64_t customCeil(double num,int64_t del)
    {
        if(static_cast<int64_t>(num) % del > 0)
        {
            num /= del;
            num++;
        } else
        {
            num /= del;
        }
        return num;
    }

__global__ void gausFilter(uint8_t *inBuffer, uint8_t *outBuffer, uint64_t width, uint64_t height, double *gausMatrix, uint64_t matrixSize)
{
    int64_t x = blockIdx.x * blockDim.x + threadIdx.x;
    int64_t y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < width && y < height)
    {
        int64_t cord = y*width+x;
        int64_t halfMatrixSize = matrixSize /  2;
        double tmpCell = 0;
        for (int64_t i = 0; i < matrixSize; i++)
        {
            for (int64_t j = 0; j < matrixSize; j++)
            {
                int64_t tmpCord = cord + width * (i - halfMatrixSize) + (j - halfMatrixSize)*3;
                if(tmpCord >= 0 && tmpCord < width*height)
                {
                    tmpCell += gausMatrix[i*matrixSize+j]*inBuffer[tmpCord];
                }
            }
        }
        outBuffer[cord] = (tmpCell > 255)? 255 : tmpCell; 
    }
}
    const int32_t block_size = 32;
    const int32_t halfGausSize = 1;
    const int32_t transactionsSize = 4;

__global__ void gausFilterGray(uint8_t *inBuffer, uint8_t *outBuffer, uint64_t width, uint64_t height, double *gausMatrix, uint64_t matrixSize)
{
    int64_t x = blockIdx.x * blockDim.x + threadIdx.x;
    int64_t y = blockIdx.y * blockDim.y + threadIdx.y;

    __shared__ char buff[(block_size + 2 * halfGausSize) * transactionsSize * (block_size + 2 * halfGausSize) * transactionsSize];
    // {
        int32_t cord = y * width + x;
        int32_t sharedCord = (y % block_size) * block_size + (x % block_size);
        int32_t length = height * width;
        if (cord >= 0 && cord < length)
        {
            reinterpret_cast<uint32_t *>(buff)[sharedCord / 4] = reinterpret_cast<uint32_t *>(inBuffer)[cord * 4];
        }
    // }
    __syncthreads();

    for (int8_t p = 0; p < transactionsSize; p++)
    {
        if (x < width && y < height)
        {
            // int32_t cord = y * width + (x+p);
            // int32_t halfMatrixSize = matrixSize /  2;
            // double tmpCell = 0;
            // for (int32_t i = 0; i < matrixSize; i++)
            // {
            //     for (int32_t j = 0; j < matrixSize; j++)
            //     {
            //         int32_t tmpCord = cord + width * (i - halfMatrixSize) + (j - halfMatrixSize)*3;
            //         if(tmpCord >= 0 && tmpCord < width*height)
            //         {
            //             tmpCell += gausMatrix[i*matrixSize+j];
            //         }
            //     }
            // }
            outBuffer[(y * width + x + p)] = buff[sharedCord + p]; 
        }
    }
}
} // namespace
#endif

#ifdef __linux__
void ImageHandler::gausFilterGpu(int64_t size, handlerType type)
{   
    int64_t bufferSize = width * height;
    if (type == handlerType::rgb)
    {
       bufferSize *= numComponents;
    }

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


    dim3 block(block_size, block_size),
        grid(customCeil(width * numComponents, block_size), customCeil(height,block_size));
        
    if(type == handlerType::rgb)
    {
        grid.x *= numComponents;
        cudaMemcpy(cudaInBuffer, pixels, bufferSize, cudaMemcpyHostToDevice);
    } else
    {
        cudaMemcpy(cudaInBuffer, grayArray, bufferSize, cudaMemcpyHostToDevice);
    }
    cudaMemcpy(gausMatrix, matrix.getMatrix(), size*size*sizeof(double), cudaMemcpyHostToDevice);	
    
    cudaEvent_t startTime;
    cudaEvent_t stopTime;
    cudaEventCreate(&startTime);
    cudaEventCreate(&stopTime);
    cudaEventRecord(startTime);
    printf("strat\n");

    if(type == handlerType::rgb)
    {
        gausFilter<<<grid, block>>>(cudaInBuffer, cudaOutBuffer, width * numComponents, height, gausMatrix, size); // without optimizations 
    } else
    {
        // gausFilter<<<grid, block>>>(cudaInBuffer, cudaOutBuffer, width, height, gausMatrix, size); // without optimizations
        {
            grid.x /= transactionsSize;
            // grid.y /= transactionsSize;
            gausFilterGray<<<grid, block>>>(cudaInBuffer, cudaOutBuffer, width, height, gausMatrix, size); // opimizated
        }
    }
    
    cudaDeviceSynchronize();
      cudaError_t error = cudaGetLastError();
    if(error != cudaSuccess)
    {
        printf("CUDA error: %s\n", cudaGetErrorString(error));
    }

    
    cudaEventRecord(stopTime);
    cudaEventSynchronize(stopTime);
    printf("stop\n");
    float resultTime;
    cudaEventElapsedTime(&resultTime, startTime, stopTime);
    printf("GPU time: %f miliseconds\n", resultTime);
    if(type == handlerType::rgb)
    {
	    cudaMemcpy(pixels, cudaOutBuffer, bufferSize, cudaMemcpyDeviceToHost);
    } else
    {
	    cudaMemcpy(grayArray, cudaOutBuffer, bufferSize, cudaMemcpyDeviceToHost);
        concatGrayComponents();
    }

    cudaFree(cudaOutBuffer); 
    cudaFree(cudaInBuffer);
    cudaFree(gausMatrix);
}
#endif

void ImageHandler::fillGrayArray()
{
    size_t length = height * width;
    for (size_t i = 0; i < length; i++)
    {
        grayArray[i] = pixels[i*numComponents];
    }
}

void ImageHandler::concatImage(uint64_t x, uint64_t y)
{
    uint64_t newlength = height * width * numComponents * x * y;
    uint8_t *newPixels = new uint8_t[newlength];
    uint64_t baseXScale = width * 3,
             xScale = baseXScale * x,
             yScale = height * y;
    for (uint64_t i = 0; i < yScale; i += height)
    {
        for (uint64_t j = 0; j < xScale; j += baseXScale)
        {
            for (uint64_t k = 0; k < height; k++)
            {
                for (uint64_t m = 0; m < baseXScale; m += numComponents)
                {
                    for (uint64_t p = 0; p < numComponents; p++)
                    {
                        uint64_t tmpNewCord = i * xScale + j + k * xScale + m + p;
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

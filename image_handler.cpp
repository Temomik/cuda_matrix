#include "image_handler.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "sub/stb_image_write.h"
#include "sub/stb_image.h"
#include <stdexcept>
#include "gaus_matrix.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include "time.h"

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

bool ImageHandler::compare(ImageHandler& other, HandlerType type) const
{
    int64_t width = std::min(this->width,other.width);
    int64_t height = std::min(this->height,other.height);
    
    int32_t length = width * height;
    if(type == HandlerType::rgb)
    {
        length *= std::min(numComponents,other.numComponents);
    }
    for (int64_t i = 0; i < length; i++)
    {
        if (pixels[i] != other.pixels[i])
        {
            return false;
        }
    }
    return true;
}

void ImageHandler::save(string outFileName) const
{
    if (!pixels)
        throw std::runtime_error("Pixels is NULL");
    stbi_write_jpg(outFileName.c_str(), width, height, numComponents, pixels, width * numComponents);
}

uint8_t ImageHandler::getGrayOneComponent(int64_t it, int64_t num) const
{
    if (it < width * height)
        return pixels[it * num];
    else
        return 0;
}

void ImageHandler::gausFilterCpu(int64_t size, HandlerType type)
{
    int64_t maxSize = width * height;
    uint8_t *switchPtr = grayArray;
    int64_t xScale = width;
    uint8_t tmpNumOfComponents = 1;
    if (type == HandlerType::rgb)
    {
        tmpNumOfComponents = numComponents;
        switchPtr = pixels;
        maxSize *= numComponents;
        xScale *= 3;
    }

    if (size <= 1)
        return;

    uint8_t *tmpPixels = new uint8_t[maxSize];
    GausMatrix matrix(size);
    int64_t twoSize = size * size;
    int64_t halfSize = size / 2;
    Time handler;
    // std::cout << "start CPU" << std::endl;
    // handler.start(ClockType::cpu);

    for (int16_t p = 0; p < twoSize; p++)
    {
        for (int64_t i = 0; i < maxSize; i++)
        {
            int16_t x = p % size - halfSize;
            int16_t y = p / size - halfSize;
            int64_t borderCheck = i % xScale + x * numComponents;
                int64_t tmpMatrixIndex = i + y * xScale + x * tmpNumOfComponents;
                if (tmpMatrixIndex >= 0 && tmpMatrixIndex < maxSize)
                    tmpPixels[i] += static_cast<double>(matrix[p]) * static_cast<double>(switchPtr[tmpMatrixIndex]);
        }
    }

    // handler.stop(ClockType::cpu);
    // std::cout << "CPU time: " << handler.getElapsed(TimeType::milliseconds) << " miliseconds" << std::endl;

    if (type == HandlerType::rgb)
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
        for (uint64_t j = 0; j < numComponents && (j + i * numComponents) < length * numComponents; j++)
        {
            pixels[i * numComponents + j] = grayArray[i];
        }
    }
}

#ifdef NVCC
namespace
{
int64_t customCeil(double num, int64_t del)
{
    if (static_cast<int64_t>(num) % del > 0)
    {
        num /= del;
        num++;
    }
    else
    {
        num /= del;
    }
    return num;
}

__global__ void gausFilter(uint8_t *inBuffer, uint8_t *outBuffer, uint64_t width, uint64_t height, double *gausMatrix, uint64_t matrixSize, uint8_t numComponents = 1)
{
    int64_t x = blockIdx.x * blockDim.x + threadIdx.x;
    int64_t y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < width && y < height)
    {
        int64_t cord = y * width + x;
        int64_t halfMatrixSize = matrixSize / 2;
        int32_t tmpCell = 0;
        for (int64_t i = 0; i < matrixSize; i++)
        {
            for (int64_t j = 0; j < matrixSize; j++)
            {
                int64_t tmpCord = cord + width * (i - halfMatrixSize) + (j - halfMatrixSize) * numComponents;
                if (tmpCord >= 0 && tmpCord < width * height)
                {
                    tmpCell += gausMatrix[i * matrixSize + j] * inBuffer[tmpCord];
                }
            }
        }
        outBuffer[cord] = (tmpCell > 255) ? 255 : tmpCell;
    }
}

const int32_t blockSize = 32;
const int32_t transactionsSize = 4;
const int32_t sharedMemorySize = blockSize * transactionsSize * blockSize * transactionsSize;

__global__ void gausFilterOptimized(uint8_t *inBuffer, uint8_t *outBuffer, uint64_t width, uint64_t height, double *gausMatrix, uint64_t matrixSize, uint16_t step = 1,uint8_t number = 0)
{
    int32_t halfGausSize = matrixSize/2;
    int32_t globalX = blockIdx.x * blockDim.x + threadIdx.x;
    int32_t globalY = blockIdx.y * blockDim.y + threadIdx.y;
    int32_t x = globalX - halfGausSize * ( 1 + 2 * blockIdx.x);
    int32_t y = globalY - halfGausSize * ( 1 + 2 * blockIdx.y);

    int32_t sharedX = globalX % blockSize + halfGausSize ;
    int32_t sharedY = globalY % blockSize + halfGausSize ;
    int32_t cord = y * width + x;
    int32_t sharedCord = sharedY * blockSize + sharedX;

    extern __shared__ uint8_t buff[];
    uint8_t transactionsBuffer[transactionsSize] = {0}; 
    // if(number == 1 || (number == 0 && cord <  (height+1) * width)|| (number == 2 && cord >=  0)|| cord >= 0 || cord <  height * width)
    if(cord >= 0 || cord <  height * width)
    {
        reinterpret_cast<uint32_t *>(buff)[sharedCord] = reinterpret_cast<uint32_t*>(inBuffer)[cord];
    } else 
    {
        reinterpret_cast<uint32_t *>(buff)[sharedCord] = *(reinterpret_cast<uint32_t*>(transactionsBuffer));
    }
    __syncthreads();

    // __shared__ double sharedGaus[9];
    // if (cord < 9)
    // {
    //     sharedGaus[cord] = gausMatrix[cord];
    // }
    // __syncthreads();

    if (x  < width && y < height &&
        globalX % blockSize >= halfGausSize &&
        globalX % blockSize < blockSize - halfGausSize &&
        globalY % blockSize >= halfGausSize &&
        globalY % blockSize < blockSize - halfGausSize)
    {
        for (int32_t p = 0; p < transactionsSize; p++)
        {
                int32_t tmpCord = cord * transactionsSize + p;
                int tmpCell = 0;
                for (int32_t i = 0; i < matrixSize; i++)
                {
                    for (int32_t j = 0; j < matrixSize; j++)
                    {
                        tmpCell += gausMatrix[i * matrixSize + j] * buff[sharedCord * transactionsSize + (i - halfGausSize) * blockSize * transactionsSize + (j - halfGausSize) * step + p];
                    }
                }
                transactionsBuffer[p] = tmpCell;
        }
        reinterpret_cast<uint32_t *>(outBuffer)[cord] = *(reinterpret_cast<uint32_t*>(transactionsBuffer));
    }
}

} // namespace
#endif

#ifdef NVCC
void ImageHandler::gausFilterGpu(int64_t size, double& elapsedTime, HandlerType type)
{
    int64_t bufferSize = width * height;
    if (type == HandlerType::rgb)
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

    if (cudaMalloc(&gausMatrix, size * size * sizeof(double)) != cudaSuccess)
    {
        cudaFree(cudaOutBuffer);
        cudaFree(cudaInBuffer);
        std::runtime_error("Fail allocate gpu memory to gausMatrix");
    }

    // if (type == HandlerType::rgb)
    // {
    //     cudaMemcpy(cudaInBuffer, pixels, bufferSize, cudaMemcpyHostToDevice);
    // }
    // else
    // {
    //     cudaMemcpy(cudaInBuffer, grayArray, bufferSize, cudaMemcpyHostToDevice);
    // }
    cudaMemcpy(gausMatrix, matrix.getMatrix(), size * size * sizeof(double), cudaMemcpyHostToDevice);

    cudaEvent_t startTime;
    cudaEvent_t stopTime;
    cudaEventCreate(&startTime);
    cudaEventCreate(&stopTime);
    cudaEventRecord(startTime);
    // printf("strat GPU\n");

    const int32_t halfGausSize = size/2;
    
        // checkCuda(cudaMemcpyAsync(&d_a[offset], &a[offset],
        //                           streamBytes, cudaMemcpyHostToDevice,
        //                           stream[i]));
    const int32_t streamCount = 3;
    int32_t tmpSize = static_cast<int32_t>(bufferSize / streamCount);
    cudaStream_t stream[streamCount];
    if (type == HandlerType::rgb)
    {
        // {
        //     dim3 block(blockSize, blockSize),
        //         grid(customCeil(width * numComponents, blockSize), customCeil(height, blockSize));
        //     gausFilter<<<grid, block>>>(cudaInBuffer, cudaOutBuffer, width * numComponents, height, gausMatrix, size,numComponents); // without optimizations
        // }
        {
            for(int32_t i = 0; i < streamCount; i++)
            {
                cudaStreamCreateWithFlags(&stream[i],cudaStreamNonBlocking);
            }


            dim3 block(blockSize, blockSize),
                grid(customCeil(width*numComponents, (blockSize - 2 * halfGausSize) * transactionsSize), customCeil(height/streamCount, blockSize - 2 * halfGausSize) );
            
            for(int32_t i = 0; i < streamCount; i++)
            {
                cudaMemcpyAsync(cudaInBuffer + tmpSize * i, pixels + tmpSize * i,tmpSize, cudaMemcpyHostToDevice, stream[i]);
                gausFilterOptimized<<<grid, block, sharedMemorySize,stream[i]>>>(cudaInBuffer + tmpSize * i, cudaOutBuffer + tmpSize * i, width * numComponents / transactionsSize, customCeil(height, streamCount), gausMatrix, size,numComponents,i); // opimizated
            }

            // for(int32_t i = 0; i < streamCount; i++)
            // {
            //     // cudaStreamSynchronize(stream[i]);
            //     cudaStreamDestroy(stream[i]);
            // }
        }
    }
    else
    {
        {
            // dim3 block(blockSize, blockSize),
            //     grid(customCeil(width, blockSize), customCeil(height,blockSize));
            // gausFilter<<<grid, block>>>(cudaInBuffer, cudaOutBuffer, width, height, gausMatrix, size); // without optimizations
        } {
            dim3 block(blockSize, blockSize),
                grid(customCeil(width, (blockSize - 2 * halfGausSize) * transactionsSize), customCeil(height, (blockSize -  2 * halfGausSize)));
            gausFilterOptimized<<<grid, block>>>(cudaInBuffer, cudaOutBuffer, width/transactionsSize, height, gausMatrix, size); // opimizated
        }
    }
    cudaError_t error = cudaGetLastError();
    if (error != cudaSuccess)
    {
        printf("CUDA error: %s\n", cudaGetErrorString(error));
        throw std::runtime_error("gpu Error");
    }

    // cudaEventRecord(stopTime);
    // cudaEventSynchronize(stopTime);
    // printf("stop\n");
    // cudaDeviceSynchronize();
    for(int32_t i = 0; i < streamCount; i++)
    {
        cudaMemcpyAsync(pixels + tmpSize * i, cudaOutBuffer + tmpSize * i, tmpSize, cudaMemcpyDeviceToHost, stream[i]);
    }
    float resultTime;
    cudaEventElapsedTime(&resultTime, startTime, stopTime);
    // printf("GPU time: %f miliseconds\n", resultTime);
    elapsedTime = resultTime;
    // if (type == HandlerType::rgb)
    // {
    //     cudaMemcpy(pixels, cudaOutBuffer, bufferSize, cudaMemcpyDeviceToHost);
    // }
    // else
    // {
    //     cudaMemcpy(grayArray, cudaOutBuffer, bufferSize, cudaMemcpyDeviceToHost);
    //     concatGrayComponents();
    // }

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
        grayArray[i] = pixels[i * numComponents];
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

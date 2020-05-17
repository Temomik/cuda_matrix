#ifndef IMAGE_HADLER
#define IMAGE_HADLER

#include <string>
#include <vector>

#define NVCC
#ifdef NVCC
// #define CUDA_API_PER_THREAD_DEFAULT_STREAM
// #define cudaStreamNonBlocking
#include "cuda_runtime.h"
#endif
using std::string;
using std::vector;

enum HandlerType
{
    gray,
    rgb
};

class ImageHandler
{
private:
    string fileName;
    uint8_t *pixels;
    uint8_t *grayArray;
    int32_t width, height, numComponents;   
    // __global__ void gausFilter(uint8_t *src, uint8_t *dst, int width, int height);
public:
    ImageHandler() = default;
    ImageHandler(string fileName, int64_t channelNum = 3);
    void load(string fileName, int64_t channelNum = 3);
    void save(string outFileName) const;
    void grayConvert();
    vector<uint8_t> getMatrix() const;
    uint8_t getGrayOneComponent(int64_t it,int64_t num) const;
    void gausFilterCpu(int64_t size,HandlerType type = HandlerType::rgb);
    void concatImage(uint64_t x, uint64_t y);
    void gausFilterGpu(int64_t size, double& elapsedTime, HandlerType type = HandlerType::rgb);
    bool compare(ImageHandler& other,HandlerType type = HandlerType::rgb) const;
    virtual ~ImageHandler();
protected:
    void concatGrayComponents();
    void fillGrayArray();
};




#endif //IMAGE_HADLER
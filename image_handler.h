#ifndef IMAGE_HADLER
#define IMAGE_HADLER

#include <string>
#include <vector>

using std::string;
using std::vector;

enum handlerType
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
    void gausFilterCpu(int64_t size,handlerType type = handlerType::rgb);
    void concatImage(uint64_t x, uint64_t y);
    void gausFilterGpu(int64_t size, handlerType type = handlerType::rgb);
    virtual ~ImageHandler();
protected:
    void concatGrayComponents();
    void fillGrayArray();
};




#endif //IMAGE_HADLER
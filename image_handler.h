#ifndef IMAGE_HADLER
#define IMAGE_HADLER

#include <string>
#include <vector>

using std::string;
using std::vector;

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
    ImageHandler(string fileName, int32_t channelNum = 3);
    void load(string fileName, int32_t channelNum = 3);
    void save(string outFileName) const;
    void grayConvert();
    vector<uint8_t> getMatrix() const;
    uint8_t getGrayElement(int32_t it) const;
    void gausFilterCpu(int32_t size);
    void concatImage(uint32_t x, uint32_t y);
    void gausFilterGpu(int32_t size);
    virtual ~ImageHandler();
protected:
    void fillGrayArray();
};




#endif //IMAGE_HADLER
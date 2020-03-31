#ifndef IMAGE_HADLER
#define IMAGE_HADLER

#include <string>

using std::string;

class ImageHandler
{
private:
    string fileName;
    uint8_t *pixels;
    int32_t width, height, numComponents;   
public:
    ImageHandler() = default;
    ImageHandler(string fileName, int32_t channelNum = 3);
    void load(string fileName, int32_t channelNum = 3);
    void save(string outFileName) const;
    void grayConvert();
    uint8_t getGrayElement(const int32_t it) const;
    void gausFilter(const int32_t size);
    virtual ~ImageHandler();
};




#endif //IMAGE_HADLER
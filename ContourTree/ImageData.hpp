#ifndef IMAGEDATA_HPP
#define IMAGEDATA_HPP

#include <string>

typedef unsigned char uchar;

struct SamplingOutput{
    std::string fileName;
    int x,y,z;
};

class ImageData
{
public:
    ImageData() {}

    static SamplingOutput writeOutput(std::string ipFolder, std::string filePrefix, int startCt, int endCt, std::string ext, std::string opFolder, std::string opPrefix, int sample, bool writeOriginal = true);
};

#endif // IMAGEDATA_HPP

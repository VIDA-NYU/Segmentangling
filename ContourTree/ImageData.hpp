#ifndef IMAGEDATA_HPP
#define IMAGEDATA_HPP

#include <QString>


struct SamplingOutput{
    QString fileName;
    int x,y,z;
};

class ImageData
{
public:
    ImageData() {}

    static SamplingOutput writeOutput(QString ipFolder, QString filePrefix, int startCt, int endCt, QString ext, QString opFolder, QString opPrefix, int sample, bool writeOriginal = true);
};

#endif // IMAGEDATA_HPP

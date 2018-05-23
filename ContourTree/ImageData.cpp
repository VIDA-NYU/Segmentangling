#include "ImageData.hpp"

//#define STBI_FAILURE_USERMSG
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

#include <iostream>
#include <fstream>
#include <vector>

#include <QImage>
#include <QDebug>
#include <QColor>

SamplingOutput ImageData::writeOutput(QString ipFolder, QString filePrefix, int startCt, int endCt, QString ext, QString opFolder, QString opPrefix, int sample, bool writeOriginal) {
    int ct = 0;
    int tmp = endCt;
    while(tmp != 0) {
        ct ++;
        tmp /= 10;
    }

    QString origRaw = opFolder + "/" + opPrefix + ".raw";
    QString origDat = opFolder + "/" + opPrefix + ".dat";

    QString sampleRaw = opFolder + "/" + opPrefix + "-sample.raw";
    QString sampleDat = opFolder + "/" + opPrefix + "-sample.dat";

    std::ofstream origf;
    std::ofstream sampf;

    if(writeOriginal) {
        origf.open(origRaw.toStdString(),std::ios::binary);
    }
    sampf.open(sampleRaw.toStdString(), std::ios::binary);

    size_t size = 0;
    std::vector<int> odata;
    std::vector<int> oct;
    std::vector<uchar> sdata;
    int sx = 0, sy = 0, sz = 0;
    int ix, iy, iz;
    iz = endCt - startCt + 1;
    for(int i = startCt;i <= endCt;i += sample) {
        for(int j = 0;j < sample;j ++) {
            int fno = i + j;
            if(fno <= endCt) {
                QString fileName = ipFolder + "/" + filePrefix + QString::number(fno).rightJustified(ct,'0') + "." + ext;
                QImage img(fileName);

                if(size == 0) {
                    size = img.width() * img.height();
                    ix = img.width();
                    iy = img.height();
                    odata.resize(size);
                    oct.resize(size);
                }
                // assuming 8-bit values
                const uchar * data = img.constBits();
                if(writeOriginal) {
                    origf.write((const char *)data,size);
                }
                for(int k = 0;k < size;k ++) {
                    if(j == 0) {
                        odata[k] = data[k];
                        oct[k] = 1;
                    } else {
                        odata[k] += data[k];
                        oct[k] ++;
                    }
                }
            }

            if(j == sample - 1) {
                for(int y = 0;y < iy;y += sample) {
                    for(int x = 0;x < ix;x += sample) {
                        int val = 0;
                        int vct = 0;
                        for(int v = 0;v < sample;v ++) {
                            for(int u = 0;u < sample;u ++) {
                                int xx = x + u;
                                int yy = y + v;
                                if(xx < ix && yy < iy) {
                                    int in = xx + yy * ix;
                                    val += odata[in];
                                    vct += oct[in];
                                }
                            }
                        }
                        val /= vct;
                        sdata.push_back(val);
                        if(i == startCt && y == 0) sx ++;
                    }
                    if(i == startCt) sy ++;
                }
            }
        }
        sz ++;
    }

    if(writeOriginal) {
        origf.close();

        std::ofstream odat;
        odat.open(origDat.toStdString());
        QString nameOnly = opPrefix + ".raw";
        odat << "RawFile: " << nameOnly.toStdString() << std::endl;
        odat << "Resolution: " << ix << " " << iy << " " << iz << std::endl;
        odat << "Format: UINT8" << std::endl;
        odat.close();
    }
    sampf.write((const char *)sdata.data(),sdata.size());
    sampf.close();

    std::ofstream sdat;
    sdat.open(sampleDat.toStdString());
    QString nameOnly = opPrefix + "-sample.raw";
    sdat << "RawFile: " << nameOnly.toStdString() << std::endl;
    sdat << "Resolution: " << sx << " " << sy << " " << sz << std::endl;
    sdat << "Format: UINT8" << std::endl;
    sdat.close();

    qDebug() << "image size" << ix << iy << iz;
    qDebug() << "sample size" << sx << sy << sz;

    SamplingOutput ret;
    ret.fileName = sampleRaw;
    ret.x = sx;
    ret.y = sy;
    ret.z = sz;
    return ret;
}


// TODO gives corrupt bmp. need to check
//        int x,y,n;
//        unsigned char *data = stbi_load(fileName.toStdString().c_str(),&x,&y,&n,0);
//        std::cout << fileName.toStdString() << std::endl;
//        std::cout << "dimensions: " << x << " " << y << std::endl;
//        std::cout << "channels: " << n << std::endl << std::endl;

//        if(data == NULL) {
//            std::cerr << "image loading failed" << std::endl;
//            std::cerr << stbi_failure_reason() << std::endl;
//        }
//        stbi_image_free(data);

//        if(i == startCt) {
//            data.resize(img.width() * img.height());
//        }
//        int ctr = 0;
//        for(int y = 0;y < img.height();y ++) {
//            for(int x = 0;x < img.width();x ++) {
//                data[ctr]= (uchar) img.pixelColor(x,y).red();
//                ctr ++;
//            }
//        }
//        origf.write((const char *)data.data(),ctr);
//        qDebug() << img.width() << img.height() << img.format();


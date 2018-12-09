#include <iostream>
#include <FreeImagePlus.h>
#include <QImage>
#include <fstream>

using namespace std;

int main()
{
    fipImage img;
    bool suc = img.load("../test.bmp");
    cout << "loaded " << suc << endl;
    const BYTE *fimg;
    if(suc) {
        cout << "width : " << img.getWidth() << endl;
        cout << "height : " << img.getHeight() << endl;
        cout << "bits / pixel: " << img.getBitsPerPixel() << endl;
        fimg = img.accessPixels();
    } else {
        return -1;
    }

    QImage qimg("../test.bmp");
    const uchar * data = qimg.constBits();
    cout << "comparing" << endl;
    int size = qimg.width() * qimg.height();
    cout << "width : " << qimg.width() << endl;
    cout << "height : " << qimg.height() << endl;

//    ofstream fo, qo;
//    fo.open("../fimg.raw",std::ios::binary);
//    fo.write((const char *)fimg,size);
//    fo.close();

//    qo.open("../qimg.raw",std::ios::binary);
//    qo.write((const char *)data,size);
//    qo.close();

    for(int x = 0;x < qimg.width();x ++) {
        for(int y = 0;y < qimg.height();y ++) {
            int qi = x + y * qimg.width();
            int fi = x + (qimg.height() - 1 - y) * qimg.width();
            if(data[qi] != fimg[fi]) {
                cerr << "error reading " << x << "," << y << " " << int(data[qi]) << " " << int(fimg[fi]) << endl;
                exit(1);
            }
        }
    }
    cout << "read correctly" << endl;

    return 0;
}

#include "Grid3DShort.hpp"

#include <cassert>
#include <fstream>
#include <QString>
#include <QDebug>

namespace contourtree {

Grid3DShort::Grid3DShort(int resx, int resy, int resz) :
    dimx(resx), dimy(resy), dimz(resz)
{
    nv = dimx * dimy * dimz;
    this->updateStars();
}

int Grid3DShort::getMaxDegree() {
    return 14;
}

int Grid3DShort::getVertexCount() {
    return nv;
}

int Grid3DShort::getStar(int64_t v, QVector<int64_t> &star) {
    int z = v / (dimx * dimy);
    int rem = v % (dimx * dimy);
    int y = rem / dimx;
    int x = rem % dimx;

    int ct = 0;
    for(int i = 0;i < 14;i ++) {
        int _x = x + starin[i][0];
        int _y = y + starin[i][1];
        int _z = z + starin[i][2];
        if(_x < 0 || _x >= dimx ||
           _y < 0 || _y >= dimy ||
           _z < 0 || _z >= dimz) {
            continue;
        }
        star[ct ++] = (v + this->star[i]);
    }
    return ct;
}

bool Grid3DShort::lessThan(int64_t v1, int64_t v2) {
    if(fnVals[v1] < fnVals[v2]) {
        return true;
    } else if(fnVals[v1] == fnVals[v2]) {
        return (v1 < v2);
    }
    return false;
}

unsigned char Grid3DShort::getFunctionValue(int64_t v) {
    return this->fnVals[v];
}

void Grid3DShort::loadGrid(QString fileName) {
    std::ifstream ip(fileName.toStdString(), std::ios::binary);
    this->fnVals.resize(nv);
    std::vector<int16_t> vals(nv);
    ip.read((char *)vals.data(),nv * sizeof(int16_t));
    ip.close();

    int16_t smax = vals[0], smin = vals[0];
    for(int i = 1;i < nv;i ++) {
        smax = std::max(smax,vals[i]);
        smin = std::min(smin,vals[i]);
    }
    qDebug() << "max, min" << smax << smin;
    unsigned char cmax = 0, cmin = 255;
    float fmax = 0, fmin = 10;
    for(int i = 1;i < nv;i ++) {
        float val = (float(vals[i] - smin)) / (smax - smin);
        unsigned char cval = (unsigned char)(val * 255);
        fnVals[i] = cval;
        cmax = std::max(cmax,cval);
        cmin = std::min(cmin,cval);
        fmax = std::max(fmax,val);
        fmin = std::min(fmin,val);
    }
    qDebug() << "cmax, cmin" << cmax << cmin;
    qDebug() << "fmax, fmin" << fmax << fmin;
}

void Grid3DShort::updateStars() {
    int ordering [][3] = {
        {0,1,2},
        {0,2,1},
        {1,0,2},
        {1,2,0},
        {2,0,1},
        {2,1,0}
    };

    Tet tet;
    tet.v[0] = index(0,0,0);
    for(int i = 0;i < 6;i ++) {
        int64_t loc[] = {0,0,0};
        for(int j = 0;j < 3;j ++) {
            loc[ordering[i][j]] ++;
            tet.v[j + 1] = index(loc[0],loc[1],loc[2]);
        }
        tets << tet;
    }


    ////
    int x = 1;
    int y = 1;
    int z = 1;
    int v = index(1,1,1);

    QSet<int64_t> unique;
    for(int xx = -1;xx < 1;xx ++) {
        int _x = x + xx;
        for(int yy = -1;yy < 1;yy ++) {
            int _y = y + yy;
            for(int zz = -1;zz < 1;zz ++) {
                int _z = z + zz;
                int vv = index(_x,_y,_z);
                for(int i = 0;i < 6;i ++) {
                    int in = -1;
                    for(int j = 0;j < 4;j ++) {
                        if(tets[i].v[j] + vv == v) {
                            in = j;
                            break;
                        }
                    }
                    if(in != -1)
                    for(int j = 0;j < 4;j ++) {
                        if(j != in) {
                            unique << (tets[i].v[j] + vv);
                        }
                    }
                }
            }
        }
    }
    assert(unique.size() == 14);

    int sct = 0;
    foreach(int64_t adj, unique) {
        z = adj / (dimx * dimy);
        int rem = adj % (dimx * dimy);
        y = rem / dimx;
        x = rem % dimx;

        starin[sct][0] = x - 1;
        starin[sct][1] = y - 1;
        starin[sct][2] = z - 1;
        star[sct] = adj - v;
        sct ++;
    }

}

} // namespace

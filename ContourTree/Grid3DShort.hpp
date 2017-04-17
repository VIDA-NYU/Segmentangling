#ifndef GRID3DSHORT_H
#define GRID3DSHORT_H

#include "ScalarFunction.hpp"
#include <QSet>
#include <stdint.h>
#include <vector>
#include "Grid3D.hpp"
namespace contourtree {

class Grid3DShort : public ScalarFunction
{
public:
    Grid3DShort(int resx, int resy, int resz);

public:
    int getMaxDegree();
    int getVertexCount();
    int getStar(int64_t v, QVector<int64_t> &star);
    bool lessThan(int64_t v1, int64_t v2);
    unsigned char getFunctionValue(int64_t v);

public:
    void loadGrid(QString fileName);

protected:
    void updateStars();

public:
    int dimx, dimy, dimz;
    int nv;
    QVector<Tet> tets;
    int starin[14][3];
    int64_t star[14];
    std::vector<unsigned char> fnVals;

protected:
    inline int64_t index(int64_t x, int64_t y, int64_t z) {
        return (x + y * dimx + z * dimx * dimy);
    }
};

} // namespace 

#endif // GRID3DSHORT_H

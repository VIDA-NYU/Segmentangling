#ifndef TRIMESH_HPP
#define TRIMESH_HPP

#include "ScalarFunction.hpp"
#include <QSet>

namespace contourtree {

class TriMesh : public ScalarFunction
{
public:
    struct Vertex {
        QSet<uint32_t> adj;
    };

public:
    TriMesh();

    int getMaxDegree();
    int getVertexCount();
    int getStar(int64_t v, QVector<int64_t> &star);
    bool lessThan(int64_t v1, int64_t v2);
    unsigned char getFunctionValue(int64_t v);

public:
    void loadData(QString fileName);

public:
    int nv;
    std::vector<unsigned char> fnVals;
    std::vector<Vertex> vertices;
    int maxStar;
};

}

#endif // TRIMESH_HPP

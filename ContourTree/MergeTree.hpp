#ifndef MERGETREE_H
#define MERGETREE_H

#include "constants.h"
#include "ScalarFunction.hpp"
#include <vector>
#include "DisjointSets.hpp"
#include <QSet>
#include "ContourTree.hpp"

namespace contourtree {

class MergeTree
{
public:
    struct Compare {
        Compare(ScalarFunction *data):data(data) {}
        bool operator () (int64_t v1, int64_t v2) {
            return data->lessThan(v1,v2);
        }

        ScalarFunction *data;
    };

public:
    MergeTree();

    void computeTree(ScalarFunction* data, TreeType type);
    void computeJoinTree();
    void computeSplitTree();
    void output(QString fileName, TreeType tree);

protected:
    void setupData();
    void orderVertices();
    void processVertex(int64_t v);
    void processVertexSplit(int64_t v);

public:
    ScalarFunction* data;
    std::vector<int64_t> cpMap;
    DisjointSets<int64_t> nodes;

    int64_t noVertices;
    int maxStar;
    std::vector<int64_t> prev;
    std::vector<int64_t> next;
    std::vector<int64_t> sv;
    std::vector<char> criticalPts;
    bool newVertex;
    int64_t newRoot;

    QSet<int64_t> set;
    ContourTree ctree;

private:
    QVector<int64_t> star;
};

}

#endif // MERGETREE_H

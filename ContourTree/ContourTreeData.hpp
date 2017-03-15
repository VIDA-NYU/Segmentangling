#ifndef CONTOURTREEDATA_HPP
#define CONTOURTREEDATA_HPP

#include <stdint.h>
#include <QVector>
#include <QHash>

namespace contourtree {

struct Node {
    QVector<uint32_t> next;
    QVector<uint32_t> prev;
};

struct Arc {
    uint32_t from;
    uint32_t to;
    uint32_t id;
};

class ContourTreeData
{
public:
    ContourTreeData();

    void loadBinFile(QString fileName);
    void loadTxtFile(QString fileName);

protected:
    void loadData(const std::vector<int64_t>& nodeids, const std::vector<unsigned char>& nodefns, const std::vector<char>& nodeTypes, const std::vector<int64_t>& iarcs);

public:
    uint32_t noNodes;
    uint32_t noArcs;

    QVector<Node> nodes;
    QVector<Arc> arcs;
    QVector<float> fnVals;
    QVector<char> type;
    QVector<int64_t> nodeVerts;

    QHash<int64_t, uint32_t> nodeMap;
};

} // namespace contourtree

#endif // CONTOURTREEDATA_HPP

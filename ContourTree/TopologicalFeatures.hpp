#ifndef TOPOLOGICALFEATURES_HPP
#define TOPOLOGICALFEATURES_HPP

#include <SimplifyCT.hpp>
#include <QString>
#include <QSet>

struct Feature {
    std::vector<uint32_t> arcs;
    uint32_t from, to;
};

class TopologicalFeatures
{
public:
    TopologicalFeatures();

    void loadData(QString dataLocation);
    std::vector<Feature> getFeatures(int topk = -1, float th = 0);

public:
    ContourTreeData ctdata;
    std::vector<uint32_t> order;
    std::vector<float> wts;
};

#endif // TOPOLOGICALFEATURES_HPP

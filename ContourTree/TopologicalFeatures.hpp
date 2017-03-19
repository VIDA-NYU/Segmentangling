#ifndef TOPOLOGICALFEATURES_HPP
#define TOPOLOGICALFEATURES_HPP

#include "SimplifyCT.hpp"
#include <QString>
#include <QSet>

namespace contourtree {

struct Feature {
    std::vector<uint32_t> arcs;
    uint32_t from, to;
};

class TopologicalFeatures
{
public:
    TopologicalFeatures();

    void loadData(QString dataLocation, bool partition = false);
    std::vector<Feature> getArcFeatures(int topk = -1, float th = 0);
    std::vector<Feature> getPartitionedExtremaFeatures(int topk = -1, float th = 0);
    std::vector<Feature> getFeatures(int topk = -1, float th = 0);

public:
    ContourTreeData ctdata;
    std::vector<uint32_t> order;
    std::vector<float> wts;

    // when completely partitioning branch decomposition
    std::vector<std::vector<uint32_t> > featureArcs;
    SimplifyCT sim;

};

}

#endif // TOPOLOGICALFEATURES_HPP

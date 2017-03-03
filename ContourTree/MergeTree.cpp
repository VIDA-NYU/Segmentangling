#include "MergeTree.hpp"

#include <chrono>
#include <QDebug>
#include <parallel/algorithm>

MergeTree::MergeTree()
{
    newRoot = 0;
}

void MergeTree::computeTree(ScalarFunction* data, MergeTree::TreeType type) {
    this->data = data;
    std::chrono::time_point<std::chrono::system_clock> ct, en;
    ct = std::chrono::system_clock::now();
    setupData();
    orderVertices();
    switch(type) {
    case ContourTree:
        qDebug() << "Not supported";
        assert(false);
        break;

    case SplitTree:
        qDebug() << "Not supported";
        assert(false);
        break;

    case JoinTree:
        computeJoinTree();
        break;

    default:
        qDebug() << "Invalid tree type";
        assert(false);
    }

    en = std::chrono::system_clock::now();
    long time = std::chrono::duration_cast<std::chrono::milliseconds>(en-ct).count();

    qDebug() << "Time taken to compute tree : " << time << "ms";
}

void MergeTree::setupData() {
    qDebug() << "setting up data";
    maxStar = data->getMaxDegree();
    noVertices = data->getVertexCount();
    newVertex = false;

    criticalPts.resize(noVertices);
    sv.resize(noVertices);
    prev.resize(noVertices,-1);
    next.resize(noVertices,-1);
    cpMap.resize(noVertices + 1);
    for(int64_t i = 0;i < noVertices;i ++) {
        sv[i] = i;
    }
    nodes = DisjointSets<int64_t>(noVertices);
}

void MergeTree::orderVertices() {
    qDebug() << "ordering vertices";
    __gnu_parallel::sort(sv.begin(),sv.end(),Compare(data));
}

void MergeTree::computeJoinTree() {
    qDebug() << "computing join tree";
    int64_t ct = 0;
    for(int64_t i = noVertices - 1;i >= 0; i --) {
        if(ct % 1000000 == 0) {
            qDebug() << "processing vertex " <<  ct << " of " << noVertices;
        }
        ct ++;

        int64_t v = sv[i];
        criticalPts[v] = REGULAR;
        processVertex(v);
    }
    int64_t in = 0;
    if(criticalPts[sv[in]] == SADDLE) {
        // add a new vertex
        newVertex = true;
    } else {
        criticalPts[sv[in]] = MINIMUM;
    }
    newRoot = in;
}

void MergeTree::output(MergeTree::TreeType type)
{
    std::vector<int64_t> arcMap;
    if(newVertex) {
        arcMap.resize(noVertices + 1,-1);
    } else {
        arcMap .resize(noVertices,-1);
    }
    int noArcs = 0;
    int noNodes = 0;
    for(int64_t i = 0;i < noVertices;i ++) {
        if(criticalPts[i] != REGULAR) {
            noNodes ++;
        }
    }
    if(newVertex) {
        noNodes ++;
    }
    noArcs = noNodes - 1;
    // Still TODO
    qDebug() << "Size of contour tree:" << noNodes << noArcs;
}

void MergeTree::processVertex(int64_t v) {
    QVector<int64_t> star = data->getStar(v);
    if(star.length() == 0) {
        return;
    }
    set.clear();
    for(int x = 0;x < star.length(); x++) {
        int64_t tin = star[x];
        if(data->lessThan(v,tin)) {
            // upperLink
            int64_t comp = nodes.find(tin);
            set << comp;
        }
    }
    if(set.size() == 0) {
        // Maximum
        int64_t comp = nodes.find(v);
        cpMap[comp] = v;
        criticalPts[v] = MAXIMUM;
    } else {
        if(set.size() > 1) {
            criticalPts[v] = SADDLE;
        }
        foreach(int64_t comp, set) {
            int64_t to = cpMap[comp];
            int64_t from = v;
            prev[to] = from;
            nodes.merge(comp, v);
        }
        int64_t comp = nodes.find(v);
        cpMap[comp] = v;
    }
}

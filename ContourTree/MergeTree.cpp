#include "MergeTree.hpp"

#include <chrono>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <fstream>

#if !defined (WIN32)
#include <parallel/algorithm>
#endif

namespace contourtree {

MergeTree::MergeTree()
{
    newRoot = 0;
}

void MergeTree::computeTree(ScalarFunction* data, TreeType type) {
    this->data = data;
    std::chrono::time_point<std::chrono::system_clock> ct, en;
    ct = std::chrono::system_clock::now();
    setupData();
    orderVertices();
    switch(type) {
    case TypeContourTree:
        computeJoinTree();
        nodes = DisjointSets<int64_t>(noVertices);
        computeSplitTree();
        ctree.setup(this);
        ctree.computeCT();
        break;

    case TypeSplitTree:
        computeSplitTree();
        break;

    case TypeJoinTree:
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
    star.resize(maxStar);

    noVertices = data->getVertexCount();
    newVertex = false;

    criticalPts.resize(noVertices, REGULAR);
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
#if defined (WIN32)
    std::sort(sv.begin(),sv.end(),Compare(data));
#else
    __gnu_parallel::sort(sv.begin(),sv.end(),Compare(data));
#endif
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


void MergeTree::computeSplitTree() {
    qDebug() << "computing split tree";
    int64_t ct = 0;
    for(int64_t i = 0;i < noVertices; i ++) {
        if(ct % 1000000 == 0) {
            qDebug() << "processing vertex " <<  ct << " of " << noVertices;
        }
        ct ++;

        int64_t v = sv[i];
        processVertexSplit(v);
    }
    int64_t in = noVertices - 1;
    if(criticalPts[sv[in]] == SADDLE) {
        // add a new vertex
        newVertex = true;
    } else {
        criticalPts[sv[in]] = MAXIMUM;
    }
    newRoot = in;
}

void MergeTree::output(QString fileName, TreeType tree)
{
    if(tree == TypeContourTree) {
        ctree.output(fileName);
        return;
    }
    // assume size of contour tree fits within 4 bytes
    std::vector<uint32_t> arcMap;
    if(newVertex) {
        arcMap.resize(noVertices + 1,-1);
    } else {
        arcMap .resize(noVertices,-1);
    }
    uint32_t noArcs = 0;
    uint32_t noNodes = 0;
    for(int64_t i = 0;i < noVertices;i ++) {
        if(criticalPts[i] != REGULAR) {
            noNodes ++;
        }
    }
    if(newVertex) {
        noNodes ++;
    }
    noArcs = noNodes - 1;

    // write meta data
    qDebug() << "Writing meta data";
    {
        QFile pr(fileName + ".rg.dat");
        if(!pr.open(QFile::WriteOnly | QIODevice::Text)) {
            qDebug() << "could not write to file" << fileName + ".rg.dat";
        }
        QTextStream text(&pr);
        text << noNodes << "\n";
        text << noArcs << "\n";
        pr.close();
    }

    qDebug() << ("Creating required memory!");
    std::vector<int64_t> nodeids(noNodes);
    std::vector<unsigned char> nodefns(noNodes);
    std::vector<char> nodeTypes(noNodes);
    std::vector<int64_t> arcs(noArcs * 2);

    std::vector<int64_t> arcFrom(noNodes);
    std::vector<int64_t> arcTo(noNodes);

    qDebug() << "Generating tree";
    int nct = 0;
    if(newVertex) {
        if(tree == TypeJoinTree){
            nodeids[nct] = noVertices;
            nodefns[nct] = 0;
            nodeTypes[nct] = MINIMUM;
            nct ++;
        }
    }
    for(int i = 0;i < noVertices;i ++) {
        if(criticalPts[sv[i]] != REGULAR) {
            nodeids[nct] = sv[i];
            nodefns[nct] = data->getFunctionValue(sv[i]);
            nodeTypes[nct] = criticalPts[sv[i]];
            nct ++;
        }
    }
    if(newVertex) {
        if(tree != TypeJoinTree){
            nodeids[nct] = noVertices;
            nodefns[nct] = 255;
            nodeTypes[nct] = MAXIMUM;
            nct ++;
        }
    }

    if(tree == TypeJoinTree) {
        uint32_t arcNo = 0;
        for(int64_t i = 0;i < noVertices;i ++) {
            if((criticalPts[i] == MAXIMUM || criticalPts[i] == SADDLE) && i != sv[0]) {
                arcMap[i] = arcNo;

                int64_t to = i;
                int64_t from = prev[to];

                while(criticalPts[from] == REGULAR) {
                    arcMap[from] = arcNo;
                    from = prev[from];
                }
                arcMap[from] = arcNo;

                arcs[arcNo * 2 + 0] = from;
                arcs[arcNo * 2 + 1] = to;
                if(criticalPts[i] == SADDLE) {
                    arcFrom[arcNo] = from;
                    arcTo[arcNo] = to;
                }
                arcNo ++;
            }
        }
        if(newVertex) {
            int to = sv[0];
            int from = noVertices;
            arcs[arcNo * 2 + 0] = from;
            arcs[arcNo * 2 + 1] = to;
            arcMap[to] = arcMap[from] = arcNo ++;
        }
        assert(arcNo == noArcs);
    } else {
        uint32_t arcNo = 0;
        for(int64_t i = 0;i < noVertices;i ++) {
            if((criticalPts[i] == MINIMUM || criticalPts[i] == SADDLE) && i != sv[sv.size() - 1]) {
                arcMap[i] = arcNo;

                int64_t from = i;
                int64_t to = next[from];

                while(criticalPts[to] == REGULAR) {
                    arcMap[to] = arcNo;
                    to = next[to];
                }
                arcMap[to] = arcNo;

                arcs[arcNo * 2 + 0] = from;
                arcs[arcNo * 2 + 1] = to;
                if(criticalPts[i] == SADDLE) {
                    arcFrom[arcNo] = from;
                    arcTo[arcNo] = to;
                }
                arcNo ++;
            }
        }
        if(newVertex) {
            int from = sv[sv.size() - 1];
            int to = noVertices;
            arcs[arcNo * 2 + 0] = from;
            arcs[arcNo * 2 + 1] = to;
            arcMap[to] = arcMap[from] = arcNo ++;
        }
        assert(arcNo == noArcs);
    }

    qDebug() << "writing tree output";
    QString rgFile = fileName + ".rg.bin";
    std::ofstream of(rgFile.toStdString(),std::ios::binary);
    of.write((char *)nodeids.data(),nodeids.size() * sizeof(int64_t));
    of.write((char *)nodefns.data(),nodeids.size());
    of.write((char *)nodeTypes.data(),nodeids.size());
    of.write((char *)arcs.data(),arcs.size() * sizeof(int64_t));
    of.close();

    qDebug() << "writing partition";
    QString rawFile = fileName + ".part.raw";
    of.open(rawFile.toStdString(), std::ios::binary);
    of.write((char *)arcMap.data(), arcMap.size() * sizeof(uint32_t));
    of.close();
}

void MergeTree::processVertex(int64_t v) {
    int starct = data->getStar(v, star);
    if(starct == 0) {
        return;
    }
    set.clear();
    for(int x = 0;x < starct; x++) {
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

void MergeTree::processVertexSplit(int64_t v) {
    int starct = data->getStar(v, star);
    if(starct == 0) {
        return;
    }
    set.clear();
    for(int x = 0;x < starct; x++) {
        int64_t tin = star[x];
        if(!(data->lessThan(v,tin))) {
            // lowerLink
            int64_t comp = nodes.find(tin);
            set << comp;
        }
    }
    if(set.size() == 0) {
        // Minimum
        int64_t comp = nodes.find(v);
        cpMap[comp] = v;
        criticalPts[v] = MINIMUM;
    } else {
        if(set.size() > 1) {
            criticalPts[v] = SADDLE;
        }
        foreach(int64_t comp, set) {
            int64_t from = cpMap[comp];
            int64_t to = v;
            next[from] = to;
            nodes.merge(comp, v);
        }
        int64_t comp = nodes.find(v);
        cpMap[comp] = v;
    }
}

}

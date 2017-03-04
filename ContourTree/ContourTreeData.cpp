#include "ContourTreeData.hpp"

#include <QFile>
#include <QTextStream>
#include <fstream>
#include <cassert>
#include <QDebug>

ContourTreeData::ContourTreeData() {

}

void ContourTreeData::loadBinFile(QString fileName) {
    // read meta data
    {
        QFile ip(fileName + ".rg.dat");
        if(!ip.open(QFile::ReadOnly | QIODevice::Text)) {
            qDebug() << "could not read file" << fileName + ".rg.dat";
        }
        QTextStream text(&ip);
        noNodes = text.readLine().toLongLong();
        noArcs = text.readLine().toLongLong();
        assert(noNodes == noArcs + 1);
        ip.close();
    }
    qDebug() << noNodes << noArcs;

    std::vector<int64_t> nodeids(noNodes);
    std::vector<unsigned char> nodefns(noNodes);
    std::vector<char> nodeTypes(noNodes);
    std::vector<int64_t> arcs(noArcs * 2);

    // read the tree
    QString rgFile = fileName + ".rg.bin";
    std::ifstream ip(rgFile.toStdString(), std::ios::binary);
    ip.read((char *)nodeids.data(),nodeids.size() * sizeof(int64_t));
    ip.read((char *)nodefns.data(),nodeids.size());
    ip.read((char *)nodeTypes.data(),nodeids.size());
    ip.read((char *)arcs.data(),arcs.size() * sizeof(int64_t));
    ip.close();

    qDebug() << "finished reading data";
    this->loadData(nodeids,nodefns,nodeTypes,arcs);
}

void ContourTreeData::loadData(const std::vector<int64_t> &nodeids, const std::vector<unsigned char> &nodefns, const std::vector<char> &nodeTypes, const std::vector<int64_t> &iarcs) {
    nodes.resize(noNodes);
    nodeVerts.resize(noNodes);
    fnVals.resize(noNodes);
    type.resize(noNodes);
    arcs.resize(noArcs);

    for(uint32_t i = 0;i < noNodes;i ++) {
        nodeVerts[i] = nodeids[i];
        fnVals[i] = (float)(nodefns[i]) / 255.;
        type[i] = nodeTypes[i];
        nodeMap[nodeVerts[i]] = i;
    }

    for(uint32_t i = 0;i < noArcs;i ++) {
        arcs[i].from = nodeMap[iarcs[i * 2 + 0]];
        arcs[i].to = nodeMap[iarcs[i * 2 + 1]];
        arcs[i].id = i;
        nodes[arcs[i].from].next << i;
        nodes[arcs[i].to].prev << i;
    }
}

#include "ContourTreeData.hpp"

#include <fstream>
#include <iostream>
#include <cassert>
#include "constants.h"

namespace contourtree {

ContourTreeData::ContourTreeData() {

}

void ContourTreeData::loadBinFile(std::string fileName) {
    // read meta data
    {
        std::ifstream ip(fileName + ".rg.dat");
//        if(!ip.open(QFile::ReadOnly | QIODevice::Text)) {
//            std::cout << "could not read file" << fileName + ".rg.dat";
//        }
//        QTextStream text(&ip);
        ip >> noNodes; // = text.readLine().toLongLong();
        ip >> noArcs; // = text.readLine().toLongLong();
        assert(noNodes == noArcs + 1);
        ip.close();
    }
    std::cout << noNodes << " " << noArcs << std::endl;

    std::vector<int64_t> nodeids(noNodes);
    std::vector<unsigned char> nodefns(noNodes);
    std::vector<char> nodeTypes(noNodes);
    std::vector<int64_t> arcs(noArcs * 2);

    // read the tree
    std::string rgFile = fileName + ".rg.bin";
    std::ifstream ip(rgFile, std::ios::binary);
    ip.read((char *)nodeids.data(),nodeids.size() * sizeof(int64_t));
    ip.read((char *)nodefns.data(),nodeids.size());
    ip.read((char *)nodeTypes.data(),nodeids.size());
    ip.read((char *)arcs.data(),arcs.size() * sizeof(int64_t));
    ip.close();

    std::cout << "finished reading data" << std::endl;
    this->loadData(nodeids,nodefns,nodeTypes,arcs);
}

void ContourTreeData::loadTxtFile(std::string fileName) {
    std::ifstream ip(fileName);
//    if(!ip.open(QFile::ReadOnly | QIODevice::Text)) {
//        std::cout << "could not read file" << fileName;
//    }
//    QTextStream text(&ip);

//    std::stringList line = text.readLine().split(" ");
//    noNodes = std::string(line[0]).toInt();
//    noArcs = std::string(line[1]).toInt();
    ip >> noNodes;
    ip >> noArcs;

    std::vector<int64_t> nodeids(noNodes);
    std::vector<unsigned char> nodefns(noNodes);
    std::vector<char> nodeTypes(noNodes);
    std::vector<int64_t> arcs(noArcs * 2);

    for(size_t i = 0;i < noNodes;i ++) {
//        line = text.readLine().split(" ");
        int64_t v;// = std::string(line[0]).toLongLong();
        float fn;//  = std::string(line[1]).toFloat();
        ip >> v;
        ip >> fn;
        char t;
        std::string type;
        ip >> type;
        if(type.compare("MINIMA") == 0) {
            t = MINIMUM;
        } else if(type.compare("MAXIMA") == 0) {
            t = MAXIMUM;
        } else if(type.compare("SADDLE") == 0) {
            t = SADDLE;
        } else {
            t = REGULAR;
        }
        nodeids[i] = v;
        nodefns[i] = (unsigned char)(fn);
        nodeTypes[i] = t;
    }
    for(size_t i = 0;i < noArcs;i ++) {
//        line = text.readLine().split(" ");
//        int v1 = std::string(line[0]).toInt();
//        int v2 = std::string(line[1]).toInt();
        int v1, v2;
        ip >> v1 >> v2;
        arcs[i * 2 + 0] = v1;
        arcs[i * 2 + 1] = v2;
    }
    ip.close();
    std::cout << "finished reading data" << std::endl;
    this->loadData(nodeids,nodefns, nodeTypes,arcs);
}

void ContourTreeData::loadData(const std::vector<int64_t> &nodeids, const std::vector<unsigned char> &nodefns, const std::vector<char> &nodeTypes, const std::vector<int64_t> &iarcs) {
    nodes.resize(noNodes);
    nodeVerts.resize(noNodes);
    fnVals.resize(noNodes);
    type.resize(noNodes);
    arcs.resize(noArcs);

    for(uint32_t i = 0;i < noNodes;i ++) {
        nodeVerts[i] = nodeids[i];
        // TODO hard coding again.
        fnVals[i] = (float)(nodefns[i]) / 255.;
        type[i] = nodeTypes[i];
        nodeMap[nodeVerts[i]] = i;
    }

    for(uint32_t i = 0;i < noArcs;i ++) {
        arcs[i].from = nodeMap[iarcs[i * 2 + 0]];
        arcs[i].to = nodeMap[iarcs[i * 2 + 1]];
        arcs[i].id = i;
        nodes[arcs[i].from].next.push_back(i);
        nodes[arcs[i].to].prev.push_back(i);
    }
}

} // namespace

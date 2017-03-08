#include "TopologicalFeatures.hpp"
#include <fstream>
#include <QFile>
#include <QTextStream>
#include <cassert>

#include<QDebug>

TopologicalFeatures::TopologicalFeatures() { }

void TopologicalFeatures::loadData(QString dataLocation) {
    ctdata = ContourTreeData();
    ctdata.loadBinFile(dataLocation);

    // read order file
    // read meta data
    QFile ip(dataLocation + ".order.dat");
    if(!ip.open(QFile::ReadOnly | QIODevice::Text)) {
        qDebug() << "could not read file" << dataLocation + ".order.dat";
    }
    QTextStream text(&ip);
    int orderSize = text.readLine().toLongLong();
    ip.close();

    order.resize(orderSize);
    wts.resize(orderSize);
    QString binFile = dataLocation + ".order.bin";
    std::ifstream bin(binFile.toStdString(), std::ios::binary);
    bin.read((char *)order.data(),order.size() * sizeof(uint32_t));
    bin.read((char *)wts.data(),wts.size() * sizeof(float));
    bin.close();
}

std::vector<Feature> TopologicalFeatures::getFeatures(int topk, float th) {
    SimplifyCT sim;
    sim.setInput(&ctdata);

    sim.simplify(order,topk,th,wts);
    std::vector<Feature> features;

    QSet<size_t> featureSet;
    for(size_t _i = 0;_i < order.size();_i ++) {
        size_t i = order.size() - _i - 1;
        if(sim.removed[order[i]]) {
            break;
        }
        featureSet << order[i];
    }
    for(size_t _i = 0;_i < order.size();_i ++) {
        size_t i = order.size() - _i - 1;
        if(sim.removed[order[i]]) {
            break;
        }
        Branch b1 = sim.branches.at(order[i]);
        Feature f;
        f.from = ctdata.nodeVerts[b1.from];
        f.to = ctdata.nodeVerts[b1.to];

        size_t bno = order[i];
        QVector<size_t> queue;
        queue << bno;
        while(queue.size() > 0) {
            size_t b = queue.at(0);
            queue.removeFirst();;
            if(b != bno && featureSet.contains(b)) {
                // this cannot happen
                assert(false);
            }
            Branch br = sim.branches.at(b);
            f.arcs.insert(f.arcs.end(),br.arcs.data(), br.arcs.data()+br.arcs.size());
            for(int i = 0;i < br.children.size();i ++) {
                int bc = br.children.at(i);
                queue.push_back(bc);
            }
        }
        features.push_back(f);
    }
    return features;
}

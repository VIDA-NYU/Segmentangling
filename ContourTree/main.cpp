#include <QCoreApplication>
#include <QDebug>

#include "DisjointSets.hpp"
#include <iostream>
#include <Grid3D.hpp>
#include <chrono>
#include <MergeTree.hpp>
#include "ContourTreeData.hpp"
#include "SimplifyCT.hpp"
#include "Persistence.hpp"
#include "TriMesh.hpp"
#include "TopologicalFeatures.hpp"
#include "HyperVolume.hpp"
#include <fstream>
#include <cmath>
#include "Grid3DShort.hpp"

using namespace contourtree;

void preProcessingShort(QString data, int dimx, int dimy, int dimz, bool persistence = true) {
    std::chrono::time_point<std::chrono::system_clock> start, end;

    Grid3DShort grid(dimx,dimy,dimz);
    start = std::chrono::system_clock::now();
    qDebug() << "loading data";
    grid.loadGrid(data + ".raw");
    MergeTree ct;
    contourtree::TreeType tree = TypeSplitTree;
    qDebug() << "computing split tree";
    ct.computeTree(&grid,tree);
    end = std::chrono::system_clock::now();
    qDebug() << "Time to compute split tree: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";
    ct.output(data, tree);

    // now simplify and store simplification hierarchy
    qDebug() << "computing segmentation hierarchy";

    start = std::chrono::system_clock::now();
    ContourTreeData ctdata;
    ctdata.loadBinFile(data);

    SimplifyCT sim;
    sim.setInput(&ctdata);
    SimFunction *simFn;
    if(persistence) {
        simFn = new Persistence(ctdata);
    } else {
        simFn = new HyperVolume(ctdata,data + ".part.raw");
    }
    sim.simplify(simFn);
    end = std::chrono::system_clock::now();
    qDebug() << "Time to simplify: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";

    sim.outputOrder(data);
    qDebug() << "done preprocessing";
}

void outputParaviewFile(QString data, int dimx, int dimy, int dimz, int noFeatures) {
    // the actual raw file without the extension. the extensions will be added as and when needed.
    qDebug() << "getting features";
    TopologicalFeatures tf;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    tf.loadData(data);
    std::vector<Feature> features = tf.getFeatures(noFeatures,0);
    end = std::chrono::system_clock::now();
    qDebug() << "Time to get features: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms";
    qDebug() << "no. of features:" << features.size() << "\n";


    qDebug() << "writing paraview file";
    QVector<uint32_t> part(dimx * dimy * dimz);
    std::ifstream ip((data + ".part.raw").toStdString(), std::ios::binary);
    ip.read((char *)(part.data()), part.size() * sizeof(uint32_t));
    ip.close();

    qDebug() << "mapping features to arcs";
    std::vector<uint32_t> arcMap(tf.ctdata.noArcs, 0);
    for(int i = 0;i < features.size();i ++) {
        for(int ano: features[i].arcs) {
            arcMap[ano] = (i + 1);
        }
    }

    qDebug() << "mapping voxels to features";
    for(int i = 0;i < part.size();i ++) {
        part[i] = arcMap[part[i]];
    }

    qDebug() << "writing output";
    std::ofstream op((data + ".test-features.raw").toStdString(), std::ios::binary);
    op.write((char *)(part.data()), part.size() * sizeof(uint32_t));
    ip.close();
    qDebug() << "done";
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString rawFile = "/home/harishd/Desktop/Projects/OCT/Data/templatep3/templatep3.raw";
    int dimx = 175;
    int dimy = 600;
    int dimz = 250;

    if(!rawFile.endsWith(".raw")) {
        qDebug() << "input should be a .raw file";
        exit(1);
    }
    // the actual raw file without the extension. the extensions will be added as and when needed.
    QString data = rawFile.left(rawFile.length() - 4);

    preProcessingShort(data,dimx,dimy,dimz);
    outputParaviewFile(data,dimx,dimy,dimz,20);

    exit(0);
    return a.exec();
}
